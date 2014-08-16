// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/quic/quic_sustained_bandwidth_recorder.h"

#include "net/quic/quic_bandwidth.h"
#include "net/quic/quic_time.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace net {
namespace test {
namespace {

TEST(QuicSustainedBandwidthRecorderTest, BandwidthEstimates) {
  QuicSustainedBandwidthRecorder recorder;
  EXPECT_FALSE(recorder.HasEstimate());

  QuicTime estimate_time = QuicTime::Zero();
  QuicWallTime wall_time = QuicWallTime::Zero();
  QuicTime::Delta srtt = QuicTime::Delta::FromMilliseconds(150);
  const int kBandwidthBitsPerSecond = 12345678;
  QuicBandwidth bandwidth =
      QuicBandwidth::FromBitsPerSecond(kBandwidthBitsPerSecond);

  // This triggers recording, but should not yield a valid estimate yet.
  recorder.RecordEstimate(true, false, bandwidth, estimate_time, wall_time,
                          srtt);
  EXPECT_FALSE(recorder.HasEstimate());

  // Send a second reading, again this should not result in a valid estimate,
  // as not enough time has passed.
  estimate_time = estimate_time.Add(srtt);
  recorder.RecordEstimate(true, false, bandwidth, estimate_time, wall_time,
                          srtt);
  EXPECT_FALSE(recorder.HasEstimate());

  // Now 3 * kSRTT has elapsed since first recording, expect a valid estimate.
  estimate_time = estimate_time.Add(srtt);
  estimate_time = estimate_time.Add(srtt);
  recorder.RecordEstimate(true, false, bandwidth, estimate_time, wall_time,
                          srtt);
  EXPECT_TRUE(recorder.HasEstimate());
  EXPECT_EQ(recorder.BandwidthEstimate(), bandwidth);
  EXPECT_EQ(recorder.BandwidthEstimate(), recorder.MaxBandwidthEstimate());

  // Resetting, and sending a different estimate will only change output after
  // a further 3 * kSRTT has passed.
  QuicBandwidth second_bandwidth =
      QuicBandwidth::FromBitsPerSecond(2 * kBandwidthBitsPerSecond);
  // Reset the recorder by passing in an unreliable measurement.
  recorder.RecordEstimate(false, false, second_bandwidth, estimate_time,
                          wall_time, srtt);
  recorder.RecordEstimate(true, false, second_bandwidth, estimate_time,
                          wall_time, srtt);
  EXPECT_EQ(recorder.BandwidthEstimate(), bandwidth);

  estimate_time = estimate_time.Add(srtt.Multiply(3));
  const int32 kSeconds = 556677;
  QuicWallTime second_bandwidth_wall_time =
      QuicWallTime::FromUNIXSeconds(kSeconds);
  recorder.RecordEstimate(true, false, second_bandwidth, estimate_time,
                          second_bandwidth_wall_time, srtt);
  EXPECT_EQ(recorder.BandwidthEstimate(), second_bandwidth);
  EXPECT_EQ(recorder.BandwidthEstimate(), recorder.MaxBandwidthEstimate());
  EXPECT_EQ(recorder.MaxBandwidthTimestamp(), kSeconds);

  // Reset again, this time recording a lower bandwidth than before.
  QuicBandwidth third_bandwidth =
      QuicBandwidth::FromBitsPerSecond(0.5 * kBandwidthBitsPerSecond);
  // Reset the recorder by passing in an unreliable measurement.
  recorder.RecordEstimate(false, false, third_bandwidth, estimate_time,
                          wall_time, srtt);
  recorder.RecordEstimate(true, false, third_bandwidth, estimate_time,
                          wall_time, srtt);
  EXPECT_EQ(recorder.BandwidthEstimate(), second_bandwidth);

  estimate_time = estimate_time.Add(srtt.Multiply(3));
  recorder.RecordEstimate(true, false, third_bandwidth, estimate_time,
                          wall_time, srtt);
  EXPECT_EQ(recorder.BandwidthEstimate(), third_bandwidth);

  // Max bandwidth should not have changed.
  EXPECT_LT(third_bandwidth, second_bandwidth);
  EXPECT_EQ(recorder.MaxBandwidthEstimate(), second_bandwidth);
  EXPECT_EQ(recorder.MaxBandwidthTimestamp(), kSeconds);
}

TEST(QuicSustainedBandwidthRecorderTest, SlowStart) {
  // Verify that slow start status is correctly recorded.
  QuicSustainedBandwidthRecorder recorder;
  EXPECT_FALSE(recorder.HasEstimate());

  QuicTime estimate_time = QuicTime::Zero();
  QuicWallTime wall_time = QuicWallTime::Zero();
  QuicTime::Delta srtt = QuicTime::Delta::FromMilliseconds(150);
  const int kBandwidthBitsPerSecond = 12345678;
  QuicBandwidth bandwidth =
      QuicBandwidth::FromBitsPerSecond(kBandwidthBitsPerSecond);

  bool in_slow_start = true;

  // This triggers recording, but should not yield a valid estimate yet.
  recorder.RecordEstimate(true, in_slow_start, bandwidth, estimate_time,
                          wall_time, srtt);

  // Now 3 * kSRTT has elapsed since first recording, expect a valid estimate.
  estimate_time = estimate_time.Add(srtt.Multiply(3));
  recorder.RecordEstimate(true, in_slow_start, bandwidth, estimate_time,
                          wall_time, srtt);
  EXPECT_TRUE(recorder.HasEstimate());
  EXPECT_TRUE(recorder.EstimateRecordedDuringSlowStart());

  // Now send another estimate, this time not in slow start.
  estimate_time = estimate_time.Add(srtt.Multiply(3));
  in_slow_start = false;
  recorder.RecordEstimate(true, in_slow_start, bandwidth, estimate_time,
                          wall_time, srtt);
  EXPECT_TRUE(recorder.HasEstimate());
  EXPECT_FALSE(recorder.EstimateRecordedDuringSlowStart());
}

}  // namespace
}  // namespace test
}  // namespace net
