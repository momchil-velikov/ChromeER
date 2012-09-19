// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_AUDIO_LINUX_AUDIO_MANAGER_LINUX_H_
#define MEDIA_AUDIO_LINUX_AUDIO_MANAGER_LINUX_H_

#include <string>
#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/threading/thread.h"
#include "media/audio/audio_manager_base.h"

namespace media {

class AlsaWrapper;

class MEDIA_EXPORT AudioManagerLinux : public AudioManagerBase {
 public:
  AudioManagerLinux();

  // Call before using a newly created AudioManagerLinux instance.
  virtual void Init() OVERRIDE;

  // Implementation of AudioManager.
  virtual bool HasAudioOutputDevices() OVERRIDE;
  virtual bool HasAudioInputDevices() OVERRIDE;
  virtual bool CanShowAudioInputSettings() OVERRIDE;
  virtual void ShowAudioInputSettings() OVERRIDE;
  virtual void GetAudioInputDeviceNames(media::AudioDeviceNames* device_names)
      OVERRIDE;

  // Implementation of AudioManagerBase.
  virtual AudioOutputStream* MakeLinearOutputStream(
      const AudioParameters& params) OVERRIDE;
  virtual AudioOutputStream* MakeLowLatencyOutputStream(
      const AudioParameters& params) OVERRIDE;
  virtual AudioInputStream* MakeLinearInputStream(
      const AudioParameters& params, const std::string& device_id) OVERRIDE;
  virtual AudioInputStream* MakeLowLatencyInputStream(
      const AudioParameters& params, const std::string& device_id) OVERRIDE;
  virtual AudioParameters GetPreferredLowLatencyOutputStreamParameters(
      const AudioParameters& input_params) OVERRIDE;

 protected:
  virtual ~AudioManagerLinux();

 private:
  enum StreamType {
    kStreamPlayback = 0,
    kStreamCapture,
  };

  // Returns true if cras should be used for input/output.
  bool UseCras();

  // Gets a list of available cras input devices.
  void GetCrasAudioInputDevices(media::AudioDeviceNames* device_names);

  // Gets a list of available ALSA input devices.
  void GetAlsaAudioInputDevices(media::AudioDeviceNames* device_names);

  // Gets the ALSA devices' names and ids.
  void GetAlsaDevicesInfo(void** hint, media::AudioDeviceNames* device_names);

  // Checks if the specific ALSA device is available.
  bool IsAlsaDeviceAvailable(const char* device_name);

  // Returns true if a device is present for the given stream type.
  bool HasAnyAlsaAudioDevice(StreamType stream);

  // Called by MakeLinearOutputStream and MakeLowLatencyOutputStream.
  AudioOutputStream* MakeOutputStream(const AudioParameters& params);

  // Called by MakeLinearInputStream and MakeLowLatencyInputStream.
  AudioInputStream* MakeInputStream(const AudioParameters& params,
                                    const std::string& device_id);

  scoped_ptr<AlsaWrapper> wrapper_;

  DISALLOW_COPY_AND_ASSIGN(AudioManagerLinux);
};

}  // namespace media

#endif  // MEDIA_AUDIO_LINUX_AUDIO_MANAGER_LINUX_H_
