// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/message_loop/message_loop.h"
#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/cancelable_task_tracker.h"
#include "chrome/browser/history/history_service.h"
#include "testing/gtest/include/gtest/gtest.h"

using base::Time;
using base::TimeDelta;

// Tests the history service for querying functionality.

namespace history {

namespace {

struct TestEntry {
  const char* url;
  const char* title;
  const int days_ago;
  Time time;  // Filled by SetUp.
} test_entries[] = {
  // This one is visited super long ago so it will be in a different database
  // from the next appearance of it at the end.
  {"http://example.com/", "Other", 180},

  // These are deliberately added out of chronological order. The history
  // service should sort them by visit time when returning query results.
  // The correct index sort order is 4 2 3 1 7 6 5 0.
  {"http://www.google.com/1", "Title PAGEONE FOO some text", 10},
  {"http://www.google.com/3", "Title PAGETHREE BAR some hello world", 8},
  {"http://www.google.com/2", "Title PAGETWO FOO some more blah blah blah", 9},

  // A more recent visit of the first one.
  {"http://example.com/", "Other", 6},

  {"http://www.google.com/6", "Title I'm the second oldest", 13},
  {"http://www.google.com/4", "Title four", 12},
  {"http://www.google.com/5", "Title five", 11},
};

// Returns true if the nth result in the given results set matches. It will
// return false on a non-match or if there aren't enough results.
bool NthResultIs(const QueryResults& results,
                 int n,  // Result index to check.
                 int test_entry_index) {  // Index of test_entries to compare.
  if (static_cast<int>(results.size()) <= n)
    return false;

  const URLResult& result = results[n];

  // Check the visit time.
  if (result.visit_time() != test_entries[test_entry_index].time)
    return false;

  // Now check the URL & title.
  return result.url() == GURL(test_entries[test_entry_index].url) &&
         result.title() ==
             base::UTF8ToUTF16(test_entries[test_entry_index].title);
}

}  // namespace

class HistoryQueryTest : public testing::Test {
 public:
  HistoryQueryTest() : page_id_(0) {
  }

  // Acts like a synchronous call to history's QueryHistory.
  void QueryHistory(const std::string& text_query,
                    const QueryOptions& options,
                    QueryResults* results) {
    history_->QueryHistory(base::UTF8ToUTF16(text_query),
                           options,
                           base::Bind(&HistoryQueryTest::QueryHistoryComplete,
                                      base::Unretained(this)),
                           &tracker_);
    // Will go until ...Complete calls Quit.
    base::MessageLoop::current()->Run();
    results->Swap(&last_query_results_);
  }

  // Test paging through results, with a fixed number of results per page.
  // Defined here so code can be shared for the text search and the non-text
  // seach versions.
  void TestPaging(const std::string& query_text,
                  const int* expected_results,
                  int results_length) {
    ASSERT_TRUE(history_.get());

    QueryOptions options;
    QueryResults results;

    options.max_count = 1;
    for (int i = 0; i < results_length; i++) {
      SCOPED_TRACE(testing::Message() << "i = " << i);
      QueryHistory(query_text, options, &results);
      ASSERT_EQ(1U, results.size());
      EXPECT_TRUE(NthResultIs(results, 0, expected_results[i]));
      options.end_time = results.back().visit_time();
    }
    QueryHistory(query_text, options, &results);
    EXPECT_EQ(0U, results.size());

    // Try with a max_count > 1.
    options.max_count = 2;
    options.end_time = base::Time();
    for (int i = 0; i < results_length / 2; i++) {
      SCOPED_TRACE(testing::Message() << "i = " << i);
      QueryHistory(query_text, options, &results);
      ASSERT_EQ(2U, results.size());
      EXPECT_TRUE(NthResultIs(results, 0, expected_results[i * 2]));
      EXPECT_TRUE(NthResultIs(results, 1, expected_results[i * 2 + 1]));
      options.end_time = results.back().visit_time();
    }

    // Add a couple of entries with duplicate timestamps. Use |query_text| as
    // the title of both entries so that they match a text query.
    TestEntry duplicates[] = {
      { "http://www.google.com/x",  query_text.c_str(), 1, },
      { "http://www.google.com/y",  query_text.c_str(), 1, }
    };
    AddEntryToHistory(duplicates[0]);
    AddEntryToHistory(duplicates[1]);

    // Make sure that paging proceeds even if there are duplicate timestamps.
    options.end_time = base::Time();
    do {
      QueryHistory(query_text, options, &results);
      ASSERT_NE(options.end_time, results.back().visit_time());
      options.end_time = results.back().visit_time();
    } while (!results.reached_beginning());
  }

 protected:
  scoped_ptr<HistoryService> history_;

  // Counter used to generate a unique ID for each page added to the history.
  int32 page_id_;

  void AddEntryToHistory(const TestEntry& entry) {
    // We need the ID scope and page ID so that the visit tracker can find it.
    ContextID context_id = reinterpret_cast<ContextID>(1);
    GURL url(entry.url);

    history_->AddPage(url, entry.time, context_id, page_id_++, GURL(),
                      history::RedirectList(), ui::PAGE_TRANSITION_LINK,
                      history::SOURCE_BROWSED, false);
    history_->SetPageTitle(url, base::UTF8ToUTF16(entry.title));
  }

 private:
  void SetUp() override {
    ASSERT_TRUE(temp_dir_.CreateUniqueTempDir());
    history_dir_ = temp_dir_.path().AppendASCII("HistoryTest");
    ASSERT_TRUE(base::CreateDirectory(history_dir_));

    history_.reset(new HistoryService);
    if (!history_->Init(history_dir_)) {
      history_.reset();  // Tests should notice this NULL ptr & fail.
      return;
    }

    // Fill the test data.
    Time now = Time::Now().LocalMidnight();
    for (size_t i = 0; i < arraysize(test_entries); i++) {
      test_entries[i].time =
          now - (test_entries[i].days_ago * TimeDelta::FromDays(1));
      AddEntryToHistory(test_entries[i]);
    }
  }

  void TearDown() override {
    if (history_) {
      history_->SetOnBackendDestroyTask(base::MessageLoop::QuitClosure());
      history_->Cleanup();
      history_.reset();
      base::MessageLoop::current()->Run();  // Wait for the other thread.
    }
  }

  void QueryHistoryComplete(QueryResults* results) {
    results->Swap(&last_query_results_);
    base::MessageLoop::current()->Quit();  // Will return out to QueryHistory.
  }

  base::ScopedTempDir temp_dir_;

  base::MessageLoop message_loop_;

  base::FilePath history_dir_;

  base::CancelableTaskTracker tracker_;

  // The QueryHistoryComplete callback will put the results here so QueryHistory
  // can return them.
  QueryResults last_query_results_;

  DISALLOW_COPY_AND_ASSIGN(HistoryQueryTest);
};

TEST_F(HistoryQueryTest, Basic) {
  ASSERT_TRUE(history_.get());

  QueryOptions options;
  QueryResults results;

  // Test duplicate collapsing. 0 is an older duplicate of 4, and should not
  // appear in the result set.
  QueryHistory(std::string(), options, &results);
  EXPECT_EQ(7U, results.size());

  EXPECT_TRUE(NthResultIs(results, 0, 4));
  EXPECT_TRUE(NthResultIs(results, 1, 2));
  EXPECT_TRUE(NthResultIs(results, 2, 3));
  EXPECT_TRUE(NthResultIs(results, 3, 1));
  EXPECT_TRUE(NthResultIs(results, 4, 7));
  EXPECT_TRUE(NthResultIs(results, 5, 6));
  EXPECT_TRUE(NthResultIs(results, 6, 5));

  // Next query a time range. The beginning should be inclusive, the ending
  // should be exclusive.
  options.begin_time = test_entries[3].time;
  options.end_time = test_entries[2].time;
  QueryHistory(std::string(), options, &results);
  EXPECT_EQ(1U, results.size());
  EXPECT_TRUE(NthResultIs(results, 0, 3));
}

// Tests max_count feature for basic (non-Full Text Search) queries.
TEST_F(HistoryQueryTest, BasicCount) {
  ASSERT_TRUE(history_.get());

  QueryOptions options;
  QueryResults results;

  // Query all time but with a limit on the number of entries. We should
  // get the N most recent entries.
  options.max_count = 2;
  QueryHistory(std::string(), options, &results);
  EXPECT_EQ(2U, results.size());
  EXPECT_TRUE(NthResultIs(results, 0, 4));
  EXPECT_TRUE(NthResultIs(results, 1, 2));
}

TEST_F(HistoryQueryTest, ReachedBeginning) {
  ASSERT_TRUE(history_.get());

  QueryOptions options;
  QueryResults results;

  QueryHistory(std::string(), options, &results);
  EXPECT_TRUE(results.reached_beginning());
  QueryHistory("some", options, &results);
  EXPECT_TRUE(results.reached_beginning());

  options.begin_time = test_entries[1].time;
  QueryHistory(std::string(), options, &results);
  EXPECT_FALSE(results.reached_beginning());
  QueryHistory("some", options, &results);
  EXPECT_FALSE(results.reached_beginning());

  // Try |begin_time| just later than the oldest visit.
  options.begin_time = test_entries[0].time + TimeDelta::FromMicroseconds(1);
  QueryHistory(std::string(), options, &results);
  EXPECT_FALSE(results.reached_beginning());
  QueryHistory("some", options, &results);
  EXPECT_FALSE(results.reached_beginning());

  // Try |begin_time| equal to the oldest visit.
  options.begin_time = test_entries[0].time;
  QueryHistory(std::string(), options, &results);
  EXPECT_TRUE(results.reached_beginning());
  QueryHistory("some", options, &results);
  EXPECT_TRUE(results.reached_beginning());

  // Try |begin_time| just earlier than the oldest visit.
  options.begin_time = test_entries[0].time - TimeDelta::FromMicroseconds(1);
  QueryHistory(std::string(), options, &results);
  EXPECT_TRUE(results.reached_beginning());
  QueryHistory("some", options, &results);
  EXPECT_TRUE(results.reached_beginning());

  // Test with |max_count| specified.
  options.max_count = 1;
  QueryHistory(std::string(), options, &results);
  EXPECT_FALSE(results.reached_beginning());
  QueryHistory("some", options, &results);
  EXPECT_FALSE(results.reached_beginning());

  // Test with |max_count| greater than the number of results,
  // and exactly equal to the number of results.
  options.max_count = 100;
  QueryHistory(std::string(), options, &results);
  EXPECT_TRUE(results.reached_beginning());
  options.max_count = results.size();
  QueryHistory(std::string(), options, &results);
  EXPECT_TRUE(results.reached_beginning());

  options.max_count = 100;
  QueryHistory("some", options, &results);
  EXPECT_TRUE(results.reached_beginning());
  options.max_count = results.size();
  QueryHistory("some", options, &results);
  EXPECT_TRUE(results.reached_beginning());
}

// This does most of the same tests above, but performs a text searches for a
// string that will match the pages in question. This will trigger a
// different code path.
TEST_F(HistoryQueryTest, TextSearch) {
  ASSERT_TRUE(history_.get());

  QueryOptions options;
  QueryResults results;

  // Query all of them to make sure they are there and in order. Note that
  // this query will return the starred item twice since we requested all
  // starred entries and no de-duping.
  QueryHistory("some", options, &results);
  EXPECT_EQ(3U, results.size());
  EXPECT_TRUE(NthResultIs(results, 0, 2));
  EXPECT_TRUE(NthResultIs(results, 1, 3));
  EXPECT_TRUE(NthResultIs(results, 2, 1));

  // Do a query that should only match one of them.
  QueryHistory("PAGETWO", options, &results);
  EXPECT_EQ(1U, results.size());
  EXPECT_TRUE(NthResultIs(results, 0, 3));

  // Next query a time range. The beginning should be inclusive, the ending
  // should be exclusive.
  options.begin_time = test_entries[1].time;
  options.end_time = test_entries[3].time;
  QueryHistory("some", options, &results);
  EXPECT_EQ(1U, results.size());
  EXPECT_TRUE(NthResultIs(results, 0, 1));
}

// Tests prefix searching for text search queries.
TEST_F(HistoryQueryTest, TextSearchPrefix) {
  ASSERT_TRUE(history_.get());

  QueryOptions options;
  QueryResults results;

  // Query with a prefix search.  Should return matches for "PAGETWO" and
  // "PAGETHREE".
  QueryHistory("PAGET", options, &results);
  EXPECT_EQ(2U, results.size());
  EXPECT_TRUE(NthResultIs(results, 0, 2));
  EXPECT_TRUE(NthResultIs(results, 1, 3));
}

// Tests max_count feature for text search queries.
TEST_F(HistoryQueryTest, TextSearchCount) {
  ASSERT_TRUE(history_.get());

  QueryOptions options;
  QueryResults results;

  // Query all time but with a limit on the number of entries. We should
  // get the N most recent entries.
  options.max_count = 2;
  QueryHistory("some", options, &results);
  EXPECT_EQ(2U, results.size());
  EXPECT_TRUE(NthResultIs(results, 0, 2));
  EXPECT_TRUE(NthResultIs(results, 1, 3));

  // Now query a subset of the pages and limit by N items. "FOO" should match
  // the 2nd & 3rd pages, but we should only get the 3rd one because of the one
  // page max restriction.
  options.max_count = 1;
  QueryHistory("FOO", options, &results);
  EXPECT_EQ(1U, results.size());
  EXPECT_TRUE(NthResultIs(results, 0, 3));
}

// Tests IDN text search by both ASCII and UTF.
TEST_F(HistoryQueryTest, TextSearchIDN) {
  ASSERT_TRUE(history_.get());

  QueryOptions options;
  QueryResults results;

  TestEntry entry = { "http://xn--d1abbgf6aiiy.xn--p1ai/",  "Nothing", 0, };
  AddEntryToHistory(entry);

  struct QueryEntry {
    std::string query;
    size_t results_size;
  } queries[] = {
    { "bad query", 0 },
    { std::string("xn--d1abbgf6aiiy.xn--p1ai"), 1 },
    { base::WideToUTF8(std::wstring(L"\u043f\u0440\u0435\u0437") +
                       L"\u0438\u0434\u0435\u043d\u0442.\u0440\u0444"), 1, },
  };

  for (size_t i = 0; i < arraysize(queries); ++i) {
    QueryHistory(queries[i].query, options, &results);
    EXPECT_EQ(queries[i].results_size, results.size());
  }
}

// Test iterating over pages of results.
TEST_F(HistoryQueryTest, Paging) {
  // Since results are fetched 1 and 2 at a time, entry #0 and #6 will not
  // be de-duplicated.
  int expected_results[] = { 4, 2, 3, 1, 7, 6, 5, 0 };
  TestPaging(std::string(), expected_results, arraysize(expected_results));
}

TEST_F(HistoryQueryTest, TextSearchPaging) {
  // Since results are fetched 1 and 2 at a time, entry #0 and #6 will not
  // be de-duplicated. Entry #4 does not contain the text "title", so it
  // shouldn't appear.
  int expected_results[] = { 2, 3, 1, 7, 6, 5 };
  TestPaging("title", expected_results, arraysize(expected_results));
}

}  // namespace history
