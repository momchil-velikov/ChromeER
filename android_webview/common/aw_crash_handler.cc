// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "android_webview/common/aw_crash_handler.h"

#include <android/log.h>
#include <signal.h>

#include "base/logging.h"

namespace {

const int kExceptionSignals[] = {
  SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS
};

struct sigaction old_handlers[arraysize(kExceptionSignals)];

bool crash_handler_registered;

std::string g_crash_msg;

const char* g_crash_msg_ptr;  // Avoid invoking STL magic in a signal handler.

void AwExceptionHandler(int sig, siginfo_t* info, void* uc) {
  if (g_crash_msg_ptr != NULL)
    __android_log_write(ANDROID_LOG_ERROR, "chromium", g_crash_msg_ptr);

  // We served our purpose. Now restore the old crash handlers. If the embedder
  // did register a custom crash handler, it will be invoked by the kernel after
  // this function returns. Otherwise, this will end up invoking the default
  // signal disposition.
  for (uint32_t i = 0; i < arraysize(kExceptionSignals); ++i) {
    if (sigaction(kExceptionSignals[i], &old_handlers[i], NULL) == -1) {
      signal(kExceptionSignals[i], SIG_DFL);
    }
  }
}

}  // namespace

namespace android_webview {
namespace crash_handler {

void RegisterCrashHandler(const std::string& version) {
  if (crash_handler_registered) {
    NOTREACHED();
    return;
  }

  g_crash_msg = "### WebView crash. " + version;
  g_crash_msg_ptr = g_crash_msg.c_str();

  // Fail if unable to store all the old handlers.
  for (uint32_t i = 0; i < arraysize(kExceptionSignals); ++i) {
    if (sigaction(kExceptionSignals[i], NULL, &old_handlers[i]) == -1) {
      LOG(ERROR) << "Error while trying to retrieve old handler for signal "
                 << kExceptionSignals[i] << ")";
      return;
    }
  }

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sigemptyset(&sa.sa_mask);

  for (uint32_t i = 0; i < arraysize(kExceptionSignals); ++i)
    sigaddset(&sa.sa_mask, kExceptionSignals[i]);

  sa.sa_sigaction = AwExceptionHandler;
  sa.sa_flags = SA_ONSTACK | SA_SIGINFO;

  for (uint32_t i = 0; i < arraysize(kExceptionSignals); ++i) {
    if (sigaction(kExceptionSignals[i], &sa, NULL) == -1) {
      // At this point it is impractical to back out changes, and so failure to
      // install a signal is intentionally ignored.
      LOG(ERROR) << "Error while overriding handler for signal "
                 << kExceptionSignals[i];
    }
  }

  crash_handler_registered = true;
}

}  // namespace crash_handler
}  // namespace android_webview
