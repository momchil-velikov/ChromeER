// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/events/touchscreen_device.h"

#include <string>

#include "ui/events/input_device.h"

namespace ui {

TouchscreenDevice::TouchscreenDevice() {
}

TouchscreenDevice::TouchscreenDevice(unsigned int id,
                                     InputDeviceType type,
                                     const std::string& name,
                                     const gfx::Size& size)
    : InputDevice(id, type, name), size(size) {
}

}  // namespace ui
