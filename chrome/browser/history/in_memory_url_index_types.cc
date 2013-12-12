// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/history/in_memory_url_index_types.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <set>

#include "base/i18n/break_iterator.h"
#include "base/i18n/case_conversion.h"
#include "base/strings/string_util.h"
#include "net/base/escape.h"
#include "net/base/net_util.h"

namespace history {

// Matches within URL and Title Strings ----------------------------------------

// The maximum length of URL or title returned by the Cleanup functions.
static const size_t kCleanedUpUrlMaxLength = 1024u;
static const size_t kCleanedUpTitleMaxLength = 1024u;

// Attempts to shorten a URL safely (i.e., by preventing the end of the URL
// from being in the middle of an escape sequence) to no more than
// kCleanedUpUrlMaxLength characters, returning the result.
std::string TruncateUrl(const std::string& url) {
  if (url.length() <= kCleanedUpUrlMaxLength)
    return url;

  // If we're in the middle of an escape sequence, truncate just before it.
  if (url[kCleanedUpUrlMaxLength - 1] == '%')
    return url.substr(0, kCleanedUpUrlMaxLength - 1);
  if (url[kCleanedUpUrlMaxLength - 2] == '%')
    return url.substr(0, kCleanedUpUrlMaxLength - 2);

  return url.substr(0, kCleanedUpUrlMaxLength);
}

base::string16 CleanUpUrlForMatching(const GURL& gurl,
                                     const std::string& languages) {
  return base::i18n::ToLower(net::FormatUrl(
      GURL(TruncateUrl(gurl.spec())), languages,
      net::kFormatUrlOmitUsernamePassword,
      net::UnescapeRule::SPACES | net::UnescapeRule::URL_SPECIAL_CHARS,
      NULL, NULL, NULL));
}

base::string16 CleanUpTitleForMatching(const base::string16& title) {
  return base::i18n::ToLower(title.substr(0u, kCleanedUpTitleMaxLength));
}

TermMatches MatchTermInString(const base::string16& term,
                              const base::string16& cleaned_string,
                              int term_num) {
  const size_t kMaxCompareLength = 2048;
  const base::string16& short_string =
      (cleaned_string.length() > kMaxCompareLength) ?
      cleaned_string.substr(0, kMaxCompareLength) : cleaned_string;
  TermMatches matches;
  for (size_t location = short_string.find(term);
       location != base::string16::npos;
       location = short_string.find(term, location + 1))
    matches.push_back(TermMatch(term_num, location, term.length()));
  return matches;
}

// Comparison function for sorting TermMatches by their offsets.
bool MatchOffsetLess(const TermMatch& m1, const TermMatch& m2) {
  return m1.offset < m2.offset;
}

TermMatches SortAndDeoverlapMatches(const TermMatches& matches) {
  if (matches.empty())
    return matches;
  TermMatches sorted_matches = matches;
  std::sort(sorted_matches.begin(), sorted_matches.end(), MatchOffsetLess);
  TermMatches clean_matches;
  TermMatch last_match;
  for (TermMatches::const_iterator iter = sorted_matches.begin();
       iter != sorted_matches.end(); ++iter) {
    if (iter->offset >= last_match.offset + last_match.length) {
      last_match = *iter;
      clean_matches.push_back(last_match);
    }
  }
  return clean_matches;
}

std::vector<size_t> OffsetsFromTermMatches(const TermMatches& matches) {
  std::vector<size_t> offsets;
  for (TermMatches::const_iterator i = matches.begin(); i != matches.end(); ++i)
    offsets.push_back(i->offset);
  return offsets;
}

TermMatches ReplaceOffsetsInTermMatches(const TermMatches& matches,
                                        const std::vector<size_t>& offsets) {
  DCHECK_EQ(matches.size(), offsets.size());
  TermMatches new_matches;
  std::vector<size_t>::const_iterator offset_iter = offsets.begin();
  for (TermMatches::const_iterator term_iter = matches.begin();
       term_iter != matches.end(); ++term_iter, ++offset_iter) {
    if (*offset_iter != base::string16::npos) {
      TermMatch new_match(*term_iter);
      new_match.offset = *offset_iter;
      new_matches.push_back(new_match);
    }
  }
  return new_matches;
}

// Utility Functions -----------------------------------------------------------

String16Set String16SetFromString16(const base::string16& cleaned_uni_string,
                                    WordStarts* word_starts) {
  String16Vector words =
      String16VectorFromString16(cleaned_uni_string, false, word_starts);
  String16Set word_set;
  for (String16Vector::const_iterator iter = words.begin(); iter != words.end();
       ++iter)
    word_set.insert(base::i18n::ToLower(*iter).substr(0, kMaxSignificantChars));
  return word_set;
}

String16Vector String16VectorFromString16(
    const base::string16& cleaned_uni_string,
    bool break_on_space,
    WordStarts* word_starts) {
  if (word_starts)
    word_starts->clear();
  base::i18n::BreakIterator iter(cleaned_uni_string,
      break_on_space ? base::i18n::BreakIterator::BREAK_SPACE :
          base::i18n::BreakIterator::BREAK_WORD);
  String16Vector words;
  if (!iter.Init())
    return words;
  while (iter.Advance()) {
    if (break_on_space || iter.IsWord()) {
      base::string16 word(iter.GetString());
      size_t initial_whitespace = 0;
      if (break_on_space) {
        base::string16 trimmed_word;
        TrimWhitespace(word, TRIM_LEADING, &trimmed_word);
        initial_whitespace = word.length() - trimmed_word.length();
        TrimWhitespace(trimmed_word, TRIM_TRAILING, &word);
      }
      if (word.empty())
        continue;
      words.push_back(word);
      if (!word_starts)
        continue;
      size_t word_start = iter.prev() + initial_whitespace;
      if (word_start < kMaxSignificantChars)
        word_starts->push_back(word_start);
    }
  }
  return words;
}

Char16Set Char16SetFromString16(const base::string16& term) {
  Char16Set characters;
  for (string16::const_iterator iter = term.begin(); iter != term.end(); ++iter)
    characters.insert(*iter);
  return characters;
}

// HistoryInfoMapValue ---------------------------------------------------------

HistoryInfoMapValue::HistoryInfoMapValue() {}
HistoryInfoMapValue::~HistoryInfoMapValue() {}

// RowWordStarts ---------------------------------------------------------------

RowWordStarts::RowWordStarts() {}
RowWordStarts::~RowWordStarts() {}

void RowWordStarts::Clear() {
  url_word_starts_.clear();
  title_word_starts_.clear();
}

}  // namespace history
