// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_BROWSER_SHELL_DEVTOOLS_DELEGATE_H_
#define CONTENT_SHELL_BROWSER_SHELL_DEVTOOLS_DELEGATE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "content/public/browser/devtools_http_handler_delegate.h"
#include "content/public/browser/devtools_manager_delegate.h"

namespace content {

class BrowserContext;
class DevToolsHttpHandler;

class ShellDevToolsDelegate : public DevToolsHttpHandlerDelegate {
 public:
  explicit ShellDevToolsDelegate(BrowserContext* browser_context);
  ~ShellDevToolsDelegate() override;

  // Stops http server.
  void Stop();

  // DevToolsHttpHandlerDelegate implementation.
  std::string GetDiscoveryPageHTML() override;
  bool BundlesFrontendResources() override;
  base::FilePath GetDebugFrontendDir() override;
  scoped_ptr<net::StreamListenSocket> CreateSocketForTethering(
      net::StreamListenSocket::Delegate* delegate,
      std::string* name) override;

  DevToolsHttpHandler* devtools_http_handler() {
    return devtools_http_handler_;
  }

 private:
  BrowserContext* browser_context_;
  DevToolsHttpHandler* devtools_http_handler_;

  DISALLOW_COPY_AND_ASSIGN(ShellDevToolsDelegate);
};

class ShellDevToolsManagerDelegate : public DevToolsManagerDelegate {
 public:
  explicit ShellDevToolsManagerDelegate(BrowserContext* browser_context);
  ~ShellDevToolsManagerDelegate() override;

  // DevToolsManagerDelegate implementation.
  void Inspect(BrowserContext* browser_context,
               DevToolsAgentHost* agent_host) override {}
  void DevToolsAgentStateChanged(DevToolsAgentHost* agent_host,
                                 bool attached) override {}
  base::DictionaryValue* HandleCommand(DevToolsAgentHost* agent_host,
                                       base::DictionaryValue* command) override;
  scoped_ptr<DevToolsTarget> CreateNewTarget(const GURL& url) override;
  void EnumerateTargets(TargetCallback callback) override;
  std::string GetPageThumbnailData(const GURL& url) override;

 private:
  BrowserContext* browser_context_;

  DISALLOW_COPY_AND_ASSIGN(ShellDevToolsManagerDelegate);
};

}  // namespace content

#endif  // CONTENT_SHELL_BROWSER_SHELL_DEVTOOLS_DELEGATE_H_
