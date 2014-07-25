// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "device/hid/hid_report_descriptor.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using namespace testing;

namespace device {

namespace {

// Digitizer descriptor from HID descriptor tool
// http://www.usb.org/developers/hidpage/dt2_4.zip
const uint8_t kDigitizer[] = {
    0x05, 0x0d,        // Usage Page (Digitizer)
    0x09, 0x01,        // Usage (0x1)
    0xa1, 0x01,        // Collection (Application)
    0x85, 0x01,        //  Report ID (0x1)
    0x09, 0x21,        //  Usage (0x21)
    0xa1, 0x00,        //  Collection (Physical)
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x30,        //   Usage (0x30)
    0x09, 0x31,        //   Usage (0x31)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x02,        //   Report Count (2)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xe0, 0x2e,  //   Logical Maximum (12000)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x0c,        //   Physical Maximum (12)
    0x65, 0x13,        //   Unit (19)
    0x55, 0x00,        //   Unit Exponent (0)
    0xa4,              //   Push
    0x81, 0x02,        //   Input (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x05, 0x0d,        //   Usage Page (Digitizer)
    0x09, 0x32,        //   Usage (0x32)
    0x09, 0x44,        //   Usage (0x44)
    0x09, 0x42,        //   Usage (0x42)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x01,        //   Physical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x03,        //   Report Count (3)
    0x65, 0x00,        //   Unit (0)
    0x81, 0x02,        //   Input (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x05,        //   Report Size (5)
    0x81, 0x03,        //   Input (Con|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0xc0,              //  End Collection
    0x85, 0x02,        //  Report ID (0x2)
    0x09, 0x20,        //  Usage (0x20)
    0xa1, 0x00,        //  Collection (Physical)
    0xb4,              //   Pop
    0xa4,              //   Push
    0x09, 0x30,        //   Usage (0x30)
    0x09, 0x31,        //   Usage (0x31)
    0x81, 0x02,        //   Input (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x05, 0x0d,        //   Usage Page (Digitizer)
    0x09, 0x32,        //   Usage (0x32)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x01,        //   Physical Maximum (1)
    0x65, 0x00,        //   Unit (0)
    0x75, 0x01,        //   Report Size (1)
    0x81, 0x02,        //   Input (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0x10,        //   Usage Maximum (16)
    0x25, 0x10,        //   Logical Maximum (16)
    0x75, 0x05,        //   Report Size (5)
    0x81, 0x40,        //   Input (Dat|Var|Rel|NoWrp|Lin|Prf|Null|BitF)
    0x75, 0x02,        //   Report Size (2)
    0x81, 0x01,        //   Input (Con|Var|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0xc0,              //  End Collection
    0x85, 0x03,        //  Report ID (0x3)
    0x05, 0x0d,        //  Usage Page (Digitizer)
    0x09, 0x20,        //  Usage (0x20)
    0xa1, 0x00,        //  Collection (Physical)
    0xb4,              //   Pop
    0x09, 0x30,        //   Usage (0x30)
    0x09, 0x31,        //   Usage (0x31)
    0x81, 0x02,        //   Input (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x05, 0x0d,        //   Usage Page (Digitizer)
    0x09, 0x32,        //   Usage (0x32)
    0x09, 0x44,        //   Usage (0x44)
    0x75, 0x01,        //   Report Size (1)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x01,        //   Physical Maximum (1)
    0x65, 0x00,        //   Unit (0)
    0x81, 0x02,        //   Input (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x95, 0x06,        //   Report Count (6)
    0x81, 0x03,        //   Input (Con|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x09, 0x30,        //   Usage (0x30)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x7f,        //   Logical Maximum (127)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x2d,        //   Physical Maximum (45)
    0x67, 0x11, 0xe1,  //   Unit (57617)
    0x00, 0x00,        //   Default
    0x55, 0x04,        //   Unit Exponent (4)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x12,        //   Input (Dat|Arr|Rel|NoWrp|NoLin|Prf|NoNull|BitF)
    0xc0,              //  End Collection
    0xc0               // End Collection
};

// Keyboard descriptor from HID descriptor tool
// http://www.usb.org/developers/hidpage/dt2_4.zip
const uint8_t kKeyboard[] = {
    0x05, 0x01,  // Usage Page (Generic Desktop)
    0x09, 0x06,  // Usage (0x6)
    0xa1, 0x01,  // Collection (Application)
    0x05, 0x07,  //  Usage Page (Keyboard)
    0x19, 0xe0,  //  Usage Minimum (224)
    0x29, 0xe7,  //  Usage Maximum (231)
    0x15, 0x00,  //  Logical Minimum (0)
    0x25, 0x01,  //  Logical Maximum (1)
    0x75, 0x01,  //  Report Size (1)
    0x95, 0x08,  //  Report Count (8)
    0x81, 0x02,  //  Input (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x95, 0x01,  //  Report Count (1)
    0x75, 0x08,  //  Report Size (8)
    0x81, 0x03,  //  Input (Con|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x95, 0x05,  //  Report Count (5)
    0x75, 0x01,  //  Report Size (1)
    0x05, 0x08,  //  Usage Page (Led)
    0x19, 0x01,  //  Usage Minimum (1)
    0x29, 0x05,  //  Usage Maximum (5)
    0x91, 0x02,  //  Output (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x95, 0x01,  //  Report Count (1)
    0x75, 0x03,  //  Report Size (3)
    0x91, 0x03,  //  Output (Con|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x95, 0x06,  //  Report Count (6)
    0x75, 0x08,  //  Report Size (8)
    0x15, 0x00,  //  Logical Minimum (0)
    0x25, 0x65,  //  Logical Maximum (101)
    0x05, 0x07,  //  Usage Page (Keyboard)
    0x19, 0x00,  //  Usage Minimum (0)
    0x29, 0x65,  //  Usage Maximum (101)
    0x81, 0x00,  //  Input (Dat|Var|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0xc0         // End Collection
};

// Monitor descriptor from HID descriptor tool
// http://www.usb.org/developers/hidpage/dt2_4.zip
const uint8_t kMonitor[] = {
    0x05, 0x80,        // Usage Page (Monitor 0)
    0x09, 0x01,        // Usage (0x1)
    0xa1, 0x01,        // Collection (Application)
    0x85, 0x01,        //  Report ID (0x1)
    0x15, 0x00,        //  Logical Minimum (0)
    0x26, 0xff, 0x00,  //  Logical Maximum (255)
    0x75, 0x08,        //  Report Size (8)
    0x95, 0x80,        //  Report Count (128)
    0x09, 0x02,        //  Usage (0x2)
    0xb2, 0x02, 0x01,  //  Feature (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|Buff)
    0x85, 0x02,        //  Report ID (0x2)
    0x95, 0xf3,        //  Report Count (243)
    0x09, 0x03,        //  Usage (0x3)
    0xb2, 0x02, 0x01,  //  Feature (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|Buff)
    0x85, 0x03,        //  Report ID (0x3)
    0x05, 0x82,        //  Usage Page (Monitor 2)
    0x95, 0x01,        //  Report Count (1)
    0x75, 0x10,        //  Report Size (16)
    0x26, 0xc8, 0x00,  //  Logical Maximum (200)
    0x09, 0x10,        //  Usage (0x10)
    0xb1, 0x02,        //  Feature (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x85, 0x04,        //  Report ID (0x4)
    0x25, 0x64,        //  Logical Maximum (100)
    0x09, 0x12,        //  Usage (0x12)
    0xb1, 0x02,        //  Feature (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x95, 0x06,        //  Report Count (6)
    0x26, 0xff, 0x00,  //  Logical Maximum (255)
    0x09, 0x16,        //  Usage (0x16)
    0x09, 0x18,        //  Usage (0x18)
    0x09, 0x1a,        //  Usage (0x1A)
    0x09, 0x6c,        //  Usage (0x6C)
    0x09, 0x6e,        //  Usage (0x6E)
    0x09, 0x70,        //  Usage (0x70)
    0xb1, 0x02,        //  Feature (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x85, 0x05,        //  Report ID (0x5)
    0x25, 0x7f,        //  Logical Maximum (127)
    0x09, 0x20,        //  Usage (0x20)
    0x09, 0x22,        //  Usage (0x22)
    0x09, 0x30,        //  Usage (0x30)
    0x09, 0x32,        //  Usage (0x32)
    0x09, 0x42,        //  Usage (0x42)
    0x09, 0x44,        //  Usage (0x44)
    0xb1, 0x02,        //  Feature (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0xc0               // End Collection
};

// Mouse descriptor from HID descriptor tool
// http://www.usb.org/developers/hidpage/dt2_4.zip
const uint8_t kMouse[] = {
    0x05, 0x01,  // Usage Page (Generic Desktop)
    0x09, 0x02,  // Usage (0x2)
    0xa1, 0x01,  // Collection (Application)
    0x09, 0x01,  //  Usage (0x1)
    0xa1, 0x00,  //  Collection (Physical)
    0x05, 0x09,  //   Usage Page (Button)
    0x19, 0x01,  //   Usage Minimum (1)
    0x29, 0x03,  //   Usage Maximum (3)
    0x15, 0x00,  //   Logical Minimum (0)
    0x25, 0x01,  //   Logical Maximum (1)
    0x95, 0x03,  //   Report Count (3)
    0x75, 0x01,  //   Report Size (1)
    0x81, 0x02,  //   Input (Dat|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x95, 0x01,  //   Report Count (1)
    0x75, 0x05,  //   Report Size (5)
    0x81, 0x03,  //   Input (Con|Arr|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x05, 0x01,  //   Usage Page (Generic Desktop)
    0x09, 0x30,  //   Usage (0x30)
    0x09, 0x31,  //   Usage (0x31)
    0x15, 0x81,  //   Logical Minimum (129)
    0x25, 0x7f,  //   Logical Maximum (127)
    0x75, 0x08,  //   Report Size (8)
    0x95, 0x02,  //   Report Count (2)
    0x81, 0x06,  //   Input (Dat|Arr|Abs|NoWrp|Lin|Prf|NoNull|BitF)
    0xc0,        //  End Collection
    0xc0         // End Collection
};

// Logitech Unifying receiver descriptor
const uint8_t kLogitechUnifyingReceiver[] = {
    0x06, 0x00, 0xFF,  // Usage Page (Vendor)
    0x09, 0x01,        // Usage (0x1)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x10,        //  Report ID (0x10)
    0x75, 0x08,        //  Report Size (8)
    0x95, 0x06,        //  Report Count (6)
    0x15, 0x00,        //  Logical Minimum (0)
    0x26, 0xFF, 0x00,  //  Logical Maximum (255)
    0x09, 0x01,        //  Usage (0x1)
    0x81, 0x00,        //  Input (Dat|Var|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x09, 0x01,        //  Usage (0x1)
    0x91, 0x00,        //  Output (Dat|Var|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0xC0,              // End Collection
    0x06, 0x00, 0xFF,  // Usage Page (Vendor)
    0x09, 0x02,        // Usage (0x2)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x11,        //  Report ID (0x11)
    0x75, 0x08,        //  Report Size (8)
    0x95, 0x13,        //  Report Count (19)
    0x15, 0x00,        //  Logical Minimum (0)
    0x26, 0xFF, 0x00,  //  Logical Maximum (255)
    0x09, 0x02,        //  Usage (0x2)
    0x81, 0x00,        //  Input (Dat|Var|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x09, 0x02,        //  Usage (0x2)
    0x91, 0x00,        //  Output (Dat|Var|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0xC0,              // End Collection
    0x06, 0x00, 0xFF,  // Usage Page (Vendor)
    0x09, 0x04,        // Usage (0x4)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x20,        //  Report ID (0x20)
    0x75, 0x08,        //  Report Size (8)
    0x95, 0x0E,        //  Report Count (14)
    0x15, 0x00,        //  Logical Minimum (0)
    0x26, 0xFF, 0x00,  //  Logical Maximum (255)
    0x09, 0x41,        //  Usage (0x41)
    0x81, 0x00,        //  Input (Dat|Var|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x09, 0x41,        //  Usage (0x41)
    0x91, 0x00,        //  Output (Dat|Var|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x85, 0x21,        //  Report ID (0x21)
    0x95, 0x1F,        //  Report Count (31)
    0x15, 0x00,        //  Logical Minimum (0)
    0x26, 0xFF, 0x00,  //  Logical Maximum (255)
    0x09, 0x42,        //  Usage (0x42)
    0x81, 0x00,        //  Input (Dat|Var|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0x09, 0x42,        //  Usage (0x42)
    0x91, 0x00,        //  Output (Dat|Var|Rel|NoWrp|Lin|Prf|NoNull|BitF)
    0xC0               // End Collection
};

}  // namespace

class HidReportDescriptorTest : public testing::Test {

 protected:
  virtual void SetUp() OVERRIDE { descriptor_ = NULL; }

  virtual void TearDown() OVERRIDE {
    if (descriptor_) {
      delete descriptor_;
    }
  }

 public:
  void ValidateDetails(
      const std::vector<HidCollectionInfo>& expected_collections,
      const int expected_max_input_report_size,
      const int expected_max_output_report_size,
      const int expected_max_feature_report_size,
      const uint8_t* bytes,
      size_t size) {
    descriptor_ = new HidReportDescriptor(bytes, size);

    std::vector<HidCollectionInfo> actual_collections;
    int actual_max_input_report_size;
    int actual_max_output_report_size;
    int actual_max_feature_report_size;
    descriptor_->GetDetails(&actual_collections,
                            &actual_max_input_report_size,
                            &actual_max_output_report_size,
                            &actual_max_feature_report_size);

    ASSERT_EQ(expected_collections.size(), actual_collections.size());

    std::vector<HidCollectionInfo>::const_iterator actual_collections_iter =
        actual_collections.begin();
    std::vector<HidCollectionInfo>::const_iterator expected_collections_iter =
        expected_collections.begin();

    while (expected_collections_iter != expected_collections.end() &&
           actual_collections_iter != actual_collections.end()) {
      HidCollectionInfo expected_collection = *expected_collections_iter;
      HidCollectionInfo actual_collection = *actual_collections_iter;

      ASSERT_EQ(expected_collection.usage.usage_page,
                actual_collection.usage.usage_page);
      ASSERT_EQ(expected_collection.usage.usage, actual_collection.usage.usage);
      ASSERT_THAT(actual_collection.report_ids,
                  ContainerEq(expected_collection.report_ids));

      expected_collections_iter++;
      actual_collections_iter++;
    }

    ASSERT_EQ(expected_max_input_report_size, actual_max_input_report_size);
    ASSERT_EQ(expected_max_output_report_size, actual_max_output_report_size);
    ASSERT_EQ(expected_max_feature_report_size, actual_max_feature_report_size);
  }

 private:
  HidReportDescriptor* descriptor_;
};

TEST_F(HidReportDescriptorTest, ValidateDetails_Digitizer) {
  HidCollectionInfo digitizer;
  digitizer.usage = HidUsageAndPage(0x01, HidUsageAndPage::kPageDigitizer);
  digitizer.report_ids.insert(1);
  digitizer.report_ids.insert(2);
  digitizer.report_ids.insert(3);
  HidCollectionInfo expected[] = {digitizer};
  ValidateDetails(std::vector<HidCollectionInfo>(
                      expected, expected + ARRAYSIZE_UNSAFE(expected)),
                  7,
                  0,
                  0,
                  kDigitizer,
                  sizeof(kDigitizer));
}

TEST_F(HidReportDescriptorTest, ValidateDetails_Keyboard) {
  HidCollectionInfo keyboard;
  keyboard.usage = HidUsageAndPage(0x06, HidUsageAndPage::kPageGenericDesktop);
  HidCollectionInfo expected[] = {keyboard};
  ValidateDetails(std::vector<HidCollectionInfo>(
                      expected, expected + ARRAYSIZE_UNSAFE(expected)),
                  8,
                  1,
                  0,
                  kKeyboard,
                  sizeof(kKeyboard));
}

TEST_F(HidReportDescriptorTest, ValidateDetails_Monitor) {
  HidCollectionInfo monitor;
  monitor.usage = HidUsageAndPage(0x01, HidUsageAndPage::kPageMonitor0);
  monitor.report_ids.insert(1);
  monitor.report_ids.insert(2);
  monitor.report_ids.insert(3);
  monitor.report_ids.insert(4);
  monitor.report_ids.insert(5);
  HidCollectionInfo expected[] = {monitor};
  ValidateDetails(std::vector<HidCollectionInfo>(
                      expected, expected + ARRAYSIZE_UNSAFE(expected)),
                  0,
                  0,
                  244,
                  kMonitor,
                  sizeof(kMonitor));
}

TEST_F(HidReportDescriptorTest, ValidateDetails_Mouse) {
  HidCollectionInfo mouse;
  mouse.usage = HidUsageAndPage(0x02, HidUsageAndPage::kPageGenericDesktop);
  HidCollectionInfo expected[] = {mouse};
  ValidateDetails(std::vector<HidCollectionInfo>(
                      expected, expected + ARRAYSIZE_UNSAFE(expected)),
                  3,
                  0,
                  0,
                  kMouse,
                  sizeof(kMouse));
}

TEST_F(HidReportDescriptorTest, ValidateDetails_LogitechUnifyingReceiver) {
  HidCollectionInfo hidpp_short;
  hidpp_short.usage = HidUsageAndPage(0x01, HidUsageAndPage::kPageVendor);
  hidpp_short.report_ids.insert(0x10);
  HidCollectionInfo hidpp_long;
  hidpp_long.usage = HidUsageAndPage(0x02, HidUsageAndPage::kPageVendor);
  hidpp_long.report_ids.insert(0x11);
  HidCollectionInfo hidpp_dj;
  hidpp_dj.usage = HidUsageAndPage(0x04, HidUsageAndPage::kPageVendor);
  hidpp_dj.report_ids.insert(0x20);
  hidpp_dj.report_ids.insert(0x21);

  HidCollectionInfo expected[] = {hidpp_short, hidpp_long, hidpp_dj};
  ValidateDetails(std::vector<HidCollectionInfo>(
                      expected, expected + ARRAYSIZE_UNSAFE(expected)),
                  32,
                  32,
                  0,
                  kLogitechUnifyingReceiver,
                  sizeof(kLogitechUnifyingReceiver));
}

}  // namespace device
