// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_BLUETOOTH_BLUETOOTH_AUDIO_SINK_H_
#define DEVICE_BLUETOOTH_BLUETOOTH_AUDIO_SINK_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"

namespace device {

// TODO(mcchou): Define a BluetoothAudioSink specific IOBuffer abstraction.

// BluetoothAudioSink represents a local A2DP audio sink where a remote device
// can stream audio data. Once a BluetoothAudioSink is successfully registered,
// user applications can obtain a pointer to a BluetoothAudioSink object via
// the interface provided by BluetoothAdapter. The validity of a
// BluetoothAudioSink depends on whether BluetoothAdapter is present and whether
// it is powered.
class BluetoothAudioSink : public base::RefCounted<BluetoothAudioSink> {
 public:
  // Possible values indicating the connection states between the
  // BluetoothAudioSink and the remote device.
  enum State {
    STATE_INVALID,  // BluetoothAdapter not presented or not powered.
    STATE_DISCONNECTED,  // Not connected.
    STATE_IDLE,  // Connected but not streaming.
    STATE_PENDING,  // Connected, streaming but not acquired.
    STATE_ACTIVE,  // Connected, streaming and acquired.
  };

  // Options to configure an A2DP audio sink.
  struct Options {
    Options();
    ~Options();

    uint8_t codec;
    std::vector<uint8_t> capabilities;
  };

  // Interface for observing changes from a BluetoothAudioSink.
  class Observer {
   public:
    virtual ~Observer() {}

    // Called when the state of the BluetoothAudioSink object is changed.
    // |audio_sink| indicates the object being changed, and |state| indicates
    // the new state of that object.
    virtual void BluetoothAudioSinkStateChanged(
        BluetoothAudioSink* audio_sink,
        BluetoothAudioSink::State state) = 0;

    // Called when the volume of the BluetoothAudioSink object is changed.
    // |audio_sink| indicates the object being changed, and |volume| indicates
    // the new volume level of that object.
    virtual void BluetoothAudioSinkVolumeChanged(
        BluetoothAudioSink* audio_sink,
        uint16_t volume) = 0;

    // TODO(mcchou): Add method to monitor the availability of audio data during
    // the streaming. This method should associate with BluetoothAudioSink
    // specific IOBuffer wrapping fd, read_mtu and write_mtu.
  };

  // The AudioSinkAcquiredCallback is used to return a BluetoothAudioSink object
  // after it is registered successfully.
  typedef base::Callback<void(
      scoped_refptr<BluetoothAudioSink>)> AudioSinkAcquiredCallback;

  // The ErrorCallback is used for the methods that can fail in which case it
  // is called.
  typedef base::Callback<void(const std::string& error_message)> ErrorCallback;

  // Adds and removes a observer for events on the BluetoothAudioSink object. If
  // monitoring multiple audio sinks, check the |audio_sink| parameter of
  // observer methods to determine which audio sink is issuing the event.
  virtual void AddObserver(Observer* observer) = 0;
  virtual void RemoveObserver(Observer* observer) = 0;

  // Getters for state and volume.
  virtual State GetState() const = 0;
  virtual uint16_t GetVolume() const = 0;

 protected:
  friend class base::RefCounted<BluetoothAudioSink>;
  BluetoothAudioSink();
  virtual ~BluetoothAudioSink();

 private:
  DISALLOW_COPY_AND_ASSIGN(BluetoothAudioSink);
};

}  // namespace device

#endif  // DEVICE_BLUETOOTH_BLUETOOTH_AUDIO_SINK_H_
