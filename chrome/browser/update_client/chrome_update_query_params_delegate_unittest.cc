// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/strings/stringprintf.h"
#include "chrome/browser/update_client/chrome_update_query_params_delegate.h"
#include "chrome/common/chrome_version_info.h"
#include "components/update_client/update_query_params.h"
#include "testing/gtest/include/gtest/gtest.h"

using base::StringPrintf;

namespace {

bool Contains(const std::string& source, const std::string& target) {
  return source.find(target) != std::string::npos;
}

}  // namespace

void TestParams(update_client::UpdateQueryParams::ProdId prod_id) {
  std::string params = update_client::UpdateQueryParams::Get(prod_id);

  EXPECT_TRUE(Contains(
      params,
      StringPrintf("os=%s", update_client::UpdateQueryParams::GetOS())));
  EXPECT_TRUE(Contains(
      params,
      StringPrintf("arch=%s", update_client::UpdateQueryParams::GetArch())));
  EXPECT_TRUE(Contains(
      params, StringPrintf(
                  "prod=%s",
                  update_client::UpdateQueryParams::GetProdIdString(prod_id))));
  EXPECT_TRUE(Contains(
      params,
      StringPrintf("prodchannel=%s",
                   ChromeUpdateQueryParamsDelegate::GetChannelString())));
  EXPECT_TRUE(Contains(
      params,
      StringPrintf("prodversion=%s", chrome::VersionInfo().Version().c_str())));
  EXPECT_TRUE(Contains(
      params,
      StringPrintf("lang=%s", ChromeUpdateQueryParamsDelegate::GetLang())));
}

TEST(ChromeUpdateQueryParamsDelegateTest, GetParams) {
  TestParams(update_client::UpdateQueryParams::CRX);
  TestParams(update_client::UpdateQueryParams::CHROME);
}
