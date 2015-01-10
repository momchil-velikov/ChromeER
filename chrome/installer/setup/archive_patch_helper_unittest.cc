// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/path_service.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/installer/setup/archive_patch_helper.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class ArchivePatchHelperTest : public testing::Test {
 protected:
  static void SetUpTestCase() {
    ASSERT_TRUE(PathService::Get(chrome::DIR_TEST_DATA, &data_dir_));
    data_dir_ = data_dir_.AppendASCII("installer");
    ASSERT_TRUE(base::PathExists(data_dir_));
  }

  static void TearDownTestCase() {
    data_dir_.clear();
  }

  virtual void SetUp() override {
    // Create a temp directory for testing.
    ASSERT_TRUE(test_dir_.CreateUniqueTempDir());
  }

  virtual void TearDown() override {
    // Clean up test directory manually so we can fail if it leaks.
    ASSERT_TRUE(test_dir_.Delete());
  }

  // The path to input data used in tests.
  static base::FilePath data_dir_;

  // The temporary directory used to contain the test operations.
  base::ScopedTempDir test_dir_;
};

base::FilePath ArchivePatchHelperTest::data_dir_;

}  // namespace

// Test that patching works.
TEST_F(ArchivePatchHelperTest, Patching) {
  base::FilePath src = data_dir_.AppendASCII("archive1.7z");
  base::FilePath patch = data_dir_.AppendASCII("archive.diff");
  base::FilePath dest = test_dir_.path().AppendASCII("archive2.7z");
  installer::ArchivePatchHelper archive_helper(test_dir_.path(),
                                               base::FilePath(),
                                               src,
                                               dest);
  archive_helper.set_last_uncompressed_file(patch);
  EXPECT_TRUE(archive_helper.EnsemblePatch() || archive_helper.BinaryPatch());
  base::FilePath base = data_dir_.AppendASCII("archive2.7z");
  EXPECT_TRUE(base::ContentsEqual(dest, base));
}
