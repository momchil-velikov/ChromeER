// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/render_frame_impl.h"

#include "content/renderer/render_view_impl.h"
#include "content/renderer/renderer_webapplicationcachehost_impl.h"

namespace content {

static RenderFrameImpl* (*g_create_render_frame_impl)(RenderViewImpl*, int32) =
    NULL;

// static
RenderFrameImpl* RenderFrameImpl::Create(
    RenderViewImpl* render_view,
    int32 routing_id) {
  DCHECK(routing_id != MSG_ROUTING_NONE);

  RenderFrameImpl* render_frame = NULL;
  if (g_create_render_frame_impl)
    render_frame = g_create_render_frame_impl(render_view, routing_id);
  else
    render_frame = new RenderFrameImpl(render_view, routing_id);

  return render_frame;
}

// static
void RenderFrameImpl::InstallCreateHook(
    RenderFrameImpl* (*create_render_frame_impl)(RenderViewImpl*, int32)) {
  CHECK(!g_create_render_frame_impl);
  g_create_render_frame_impl = create_render_frame_impl;
}

RenderFrameImpl::RenderFrameImpl(RenderViewImpl* render_view, int routing_id)
    : render_view_(render_view),
      routing_id_(routing_id) {
}

RenderFrameImpl::~RenderFrameImpl() {
}

bool RenderFrameImpl::Send(IPC::Message* message) {
  // TODO(nasko): Move away from using the RenderView's Send method once we
  // have enough infrastructure and state to make the right checks here.
  return render_view_->Send(message);
}

bool RenderFrameImpl::OnMessageReceived(const IPC::Message& msg) {
  // Pass the message up to the RenderView, until we have enough
  // infrastructure to start processing messages in this object.
  return render_view_->OnMessageReceived(msg);
}

// WebKit::WebFrameClient implementation -------------------------------------

WebKit::WebPlugin* RenderFrameImpl::createPlugin(
    WebKit::WebFrame* frame,
    const WebKit::WebPluginParams& params) {
  return render_view_->createPlugin(frame, params);
}

WebKit::WebSharedWorker* RenderFrameImpl::createSharedWorker(
    WebKit::WebFrame* frame,
    const WebKit::WebURL& url,
    const WebKit::WebString& name,
    unsigned long long document_id) {
  return render_view_->createSharedWorker(frame, url, name, document_id);
}

WebKit::WebMediaPlayer* RenderFrameImpl::createMediaPlayer(
    WebKit::WebFrame* frame,
    const WebKit::WebURL& url,
    WebKit::WebMediaPlayerClient* client) {
  return render_view_->createMediaPlayer(frame, url, client);
}

WebKit::WebApplicationCacheHost* RenderFrameImpl::createApplicationCacheHost(
    WebKit::WebFrame* frame,
    WebKit::WebApplicationCacheHostClient* client) {
  return render_view_->createApplicationCacheHost(frame, client);
}

WebKit::WebCookieJar* RenderFrameImpl::cookieJar(WebKit::WebFrame* frame) {
  return render_view_->cookieJar(frame);
}

void RenderFrameImpl::didAccessInitialDocument(WebKit::WebFrame* frame) {
  render_view_->didAccessInitialDocument(frame);
}

void RenderFrameImpl::didCreateFrame(WebKit::WebFrame* parent,
                                     WebKit::WebFrame* child) {
  render_view_->didCreateFrame(parent, child);
}

void RenderFrameImpl::didDisownOpener(WebKit::WebFrame* frame) {
  render_view_->didDisownOpener(frame);
}

void RenderFrameImpl::frameDetached(WebKit::WebFrame* frame) {
  render_view_->frameDetached(frame);
}

void RenderFrameImpl::willClose(WebKit::WebFrame* frame) {
  render_view_->willClose(frame);
}

void RenderFrameImpl::didChangeName(WebKit::WebFrame* frame,
                                    const WebKit::WebString& name) {
  render_view_->didChangeName(frame, name);
}

void RenderFrameImpl::loadURLExternally(WebKit::WebFrame* frame,
                                        const WebKit::WebURLRequest& request,
                                        WebKit::WebNavigationPolicy policy) {
  render_view_->loadURLExternally(frame, request, policy);
}

void RenderFrameImpl::loadURLExternally(
    WebKit::WebFrame* frame,
    const WebKit::WebURLRequest& request,
    WebKit::WebNavigationPolicy policy,
    const WebKit::WebString& suggested_name) {
  render_view_->loadURLExternally(frame, request, policy, suggested_name);
}

WebKit::WebNavigationPolicy RenderFrameImpl::decidePolicyForNavigation(
    WebKit::WebFrame* frame,
    const WebKit::WebURLRequest& request,
    WebKit::WebNavigationType type,
    WebKit::WebNavigationPolicy default_policy,
    bool is_redirect) {
  return render_view_->decidePolicyForNavigation(
      frame, request, type, default_policy, is_redirect);
}

void RenderFrameImpl::willSendSubmitEvent(WebKit::WebFrame* frame,
                                          const WebKit::WebFormElement& form) {
  render_view_->willSendSubmitEvent(frame, form);
}

void RenderFrameImpl::willSubmitForm(WebKit::WebFrame* frame,
                                     const WebKit::WebFormElement& form) {
  render_view_->willSubmitForm(frame, form);
}

void RenderFrameImpl::willPerformClientRedirect(WebKit::WebFrame* frame,
                                                const WebKit::WebURL& from,
                                                const WebKit::WebURL& to,
                                                double interval,
                                                double fire_time) {
  render_view_->willPerformClientRedirect(frame, from, to, interval, fire_time);
}

void RenderFrameImpl::didCancelClientRedirect(WebKit::WebFrame* frame) {
  render_view_->didCancelClientRedirect(frame);
}

void RenderFrameImpl::didCompleteClientRedirect(WebKit::WebFrame* frame,
                                                const WebKit::WebURL& from) {
  render_view_->didCompleteClientRedirect(frame, from);
}

void RenderFrameImpl::didCreateDataSource(WebKit::WebFrame* frame,
                                          WebKit::WebDataSource* datasource) {
  render_view_->didCreateDataSource(frame, datasource);
}

void RenderFrameImpl::didStartProvisionalLoad(WebKit::WebFrame* frame) {
  render_view_->didStartProvisionalLoad(frame);
}

void RenderFrameImpl::didReceiveServerRedirectForProvisionalLoad(
    WebKit::WebFrame* frame) {
  render_view_->didReceiveServerRedirectForProvisionalLoad(frame);
}

void RenderFrameImpl::didFailProvisionalLoad(
    WebKit::WebFrame* frame,
    const WebKit::WebURLError& error) {
  render_view_->didFailProvisionalLoad(frame, error);
}

void RenderFrameImpl::didCommitProvisionalLoad(WebKit::WebFrame* frame,
                                               bool is_new_navigation) {
  render_view_->didCommitProvisionalLoad(frame, is_new_navigation);
}

void RenderFrameImpl::didClearWindowObject(WebKit::WebFrame* frame) {
  render_view_->didClearWindowObject(frame);
}

void RenderFrameImpl::didCreateDocumentElement(WebKit::WebFrame* frame) {
  render_view_->didCreateDocumentElement(frame);
}

void RenderFrameImpl::didReceiveTitle(WebKit::WebFrame* frame,
                                      const WebKit::WebString& title,
                                      WebKit::WebTextDirection direction) {
  render_view_->didReceiveTitle(frame, title, direction);
}

void RenderFrameImpl::didChangeIcon(WebKit::WebFrame* frame,
                                    WebKit::WebIconURL::Type icon_type) {
  render_view_->didChangeIcon(frame, icon_type);
}

void RenderFrameImpl::didFinishDocumentLoad(WebKit::WebFrame* frame) {
  render_view_->didFinishDocumentLoad(frame);
}

void RenderFrameImpl::didHandleOnloadEvents(WebKit::WebFrame* frame) {
  render_view_->didHandleOnloadEvents(frame);
}

void RenderFrameImpl::didFailLoad(WebKit::WebFrame* frame,
                                  const WebKit::WebURLError& error) {
  render_view_->didFailLoad(frame, error);
}

void RenderFrameImpl::didFinishLoad(WebKit::WebFrame* frame) {
  render_view_->didFinishLoad(frame);
}

void RenderFrameImpl::didNavigateWithinPage(WebKit::WebFrame* frame,
                                            bool is_new_navigation) {
  render_view_->didNavigateWithinPage(frame, is_new_navigation);
}

void RenderFrameImpl::didUpdateCurrentHistoryItem(WebKit::WebFrame* frame) {
  render_view_->didUpdateCurrentHistoryItem(frame);
}

void RenderFrameImpl::willSendRequest(
    WebKit::WebFrame* frame,
    unsigned identifier,
    WebKit::WebURLRequest& request,
    const WebKit::WebURLResponse& redirect_response) {
  render_view_->willSendRequest(frame, identifier, request, redirect_response);
}

void RenderFrameImpl::didReceiveResponse(
    WebKit::WebFrame* frame,
    unsigned identifier,
    const WebKit::WebURLResponse& response) {
  render_view_->didReceiveResponse(frame, identifier, response);
}

void RenderFrameImpl::didFinishResourceLoad(WebKit::WebFrame* frame,
                                            unsigned identifier) {
  render_view_->didFinishResourceLoad(frame, identifier);
}

void RenderFrameImpl::didLoadResourceFromMemoryCache(
    WebKit::WebFrame* frame,
    const WebKit::WebURLRequest& request,
    const WebKit::WebURLResponse& response) {
  render_view_->didLoadResourceFromMemoryCache(frame, request, response);
}

void RenderFrameImpl::didDisplayInsecureContent(WebKit::WebFrame* frame) {
  render_view_->didDisplayInsecureContent(frame);
}

void RenderFrameImpl::didRunInsecureContent(
    WebKit::WebFrame* frame,
    const WebKit::WebSecurityOrigin& origin,
    const WebKit::WebURL& target) {
  render_view_->didRunInsecureContent(frame, origin, target);
}

void RenderFrameImpl::didExhaustMemoryAvailableForScript(
    WebKit::WebFrame* frame) {
  render_view_->didExhaustMemoryAvailableForScript(frame);
}

void RenderFrameImpl::didCreateScriptContext(WebKit::WebFrame* frame,
                                             v8::Handle<v8::Context> context,
                                             int extension_group,
                                             int world_id) {
  render_view_->didCreateScriptContext(
      frame, context, extension_group, world_id);
}

void RenderFrameImpl::willReleaseScriptContext(WebKit::WebFrame* frame,
                                               v8::Handle<v8::Context> context,
                                               int world_id) {
  render_view_->willReleaseScriptContext(frame, context, world_id);
}

void RenderFrameImpl::didFirstVisuallyNonEmptyLayout(WebKit::WebFrame* frame) {
  render_view_->didFirstVisuallyNonEmptyLayout(frame);
}

void RenderFrameImpl::didChangeContentsSize(WebKit::WebFrame* frame,
                                            const WebKit::WebSize& size) {
  render_view_->didChangeContentsSize(frame, size);
}

void RenderFrameImpl::didChangeScrollOffset(WebKit::WebFrame* frame) {
  render_view_->didChangeScrollOffset(frame);
}

void RenderFrameImpl::willInsertBody(WebKit::WebFrame* frame) {
  render_view_->willInsertBody(frame);
}

void RenderFrameImpl::reportFindInPageMatchCount(int request_id,
                                                 int count,
                                                 bool final_update) {
  render_view_->reportFindInPageMatchCount(request_id, count, final_update);
}

void RenderFrameImpl::reportFindInPageSelection(int request_id,
                                                int active_match_ordinal,
                                                const WebKit::WebRect& sel) {
  render_view_->reportFindInPageSelection(
      request_id, active_match_ordinal, sel);
}

void RenderFrameImpl::openFileSystem(
    WebKit::WebFrame* frame,
    WebKit::WebFileSystemType type,
    long long size,
    bool create,
    WebKit::WebFileSystemCallbacks* callbacks) {
  render_view_->openFileSystem(frame, type, size, create, callbacks);
}

void RenderFrameImpl::deleteFileSystem(
    WebKit::WebFrame* frame,
    WebKit::WebFileSystemType type,
    WebKit::WebFileSystemCallbacks* callbacks) {
  render_view_->deleteFileSystem(frame, type, callbacks);
}

void RenderFrameImpl::queryStorageUsageAndQuota(
    WebKit::WebFrame* frame,
    WebKit::WebStorageQuotaType type,
    WebKit::WebStorageQuotaCallbacks* callbacks) {
  render_view_->queryStorageUsageAndQuota(frame, type, callbacks);
}

void RenderFrameImpl::requestStorageQuota(
    WebKit::WebFrame* frame,
    WebKit::WebStorageQuotaType type,
    unsigned long long requested_size,
    WebKit::WebStorageQuotaCallbacks* callbacks) {
  render_view_->requestStorageQuota(frame, type, requested_size, callbacks);
}

void RenderFrameImpl::willOpenSocketStream(
    WebKit::WebSocketStreamHandle* handle) {
  render_view_->willOpenSocketStream(handle);
}

void RenderFrameImpl::willStartUsingPeerConnectionHandler(
    WebKit::WebFrame* frame,
    WebKit::WebRTCPeerConnectionHandler* handler) {
  render_view_->willStartUsingPeerConnectionHandler(frame, handler);
}

bool RenderFrameImpl::willCheckAndDispatchMessageEvent(
    WebKit::WebFrame* sourceFrame,
    WebKit::WebFrame* targetFrame,
    WebKit::WebSecurityOrigin targetOrigin,
    WebKit::WebDOMMessageEvent event) {
  return render_view_->willCheckAndDispatchMessageEvent(
      sourceFrame, targetFrame, targetOrigin, event);
}

WebKit::WebString RenderFrameImpl::userAgentOverride(
    WebKit::WebFrame* frame,
    const WebKit::WebURL& url) {
  return render_view_->userAgentOverride(frame, url);
}

WebKit::WebString RenderFrameImpl::doNotTrackValue(WebKit::WebFrame* frame) {
  return render_view_->doNotTrackValue(frame);
}

bool RenderFrameImpl::allowWebGL(WebKit::WebFrame* frame, bool default_value) {
  return render_view_->allowWebGL(frame, default_value);
}

void RenderFrameImpl::didLoseWebGLContext(WebKit::WebFrame* frame,
                                          int arb_robustness_status_code) {
  render_view_->didLoseWebGLContext(frame, arb_robustness_status_code);
}

}  // namespace content
