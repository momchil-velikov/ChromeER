// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_WM_PUBLIC_DISPATCHER_CLIENT_H_
#define UI_WM_PUBLIC_DISPATCHER_CLIENT_H_

#include "base/message_loop/message_pump_dispatcher.h"
#include "ui/wm/wm_export.h"

namespace aura {
class Window;
namespace client {

// An interface implemented by an object which handles nested dispatchers.
class WM_EXPORT DispatcherClient {
 public:
  virtual void RunWithDispatcher(base::MessagePumpDispatcher* dispatcher) = 0;

  virtual void QuitNestedMessageLoop() = 0;
};

WM_EXPORT void SetDispatcherClient(Window* root_window,
                                   DispatcherClient* client);
WM_EXPORT DispatcherClient* GetDispatcherClient(Window* root_window);

}  // namespace client
}  // namespace aura

#endif  // UI_WM_PUBLIC_DISPATCHER_CLIENT_H_
