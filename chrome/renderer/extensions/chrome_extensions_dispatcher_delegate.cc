// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/renderer/extensions/chrome_extensions_dispatcher_delegate.h"

#include "base/command_line.h"
#include "base/sha1.h"
#include "base/strings/string_number_conversions.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/chrome_version_info.h"
#include "chrome/common/crash_keys.h"
#include "chrome/common/extensions/features/feature_channel.h"
#include "chrome/common/url_constants.h"
#include "chrome/grit/renderer_resources.h"
#include "chrome/renderer/extensions/app_bindings.h"
#include "chrome/renderer/extensions/automation_internal_custom_bindings.h"
#include "chrome/renderer/extensions/chrome_v8_context.h"
#include "chrome/renderer/extensions/enterprise_platform_keys_natives.h"
#include "chrome/renderer/extensions/file_browser_handler_custom_bindings.h"
#include "chrome/renderer/extensions/file_manager_private_custom_bindings.h"
#include "chrome/renderer/extensions/media_galleries_custom_bindings.h"
#include "chrome/renderer/extensions/notifications_native_handler.h"
#include "chrome/renderer/extensions/page_capture_custom_bindings.h"
#include "chrome/renderer/extensions/sync_file_system_custom_bindings.h"
#include "chrome/renderer/extensions/tab_finder.h"
#include "chrome/renderer/extensions/tabs_custom_bindings.h"
#include "chrome/renderer/extensions/webstore_bindings.h"
#include "content/public/renderer/render_thread.h"
#include "content/public/renderer/render_view.h"
#include "extensions/common/extension.h"
#include "extensions/common/feature_switch.h"
#include "extensions/common/permissions/api_permission_set.h"
#include "extensions/common/permissions/manifest_permission_set.h"
#include "extensions/common/permissions/permission_set.h"
#include "extensions/common/permissions/permissions_data.h"
#include "extensions/common/switches.h"
#include "extensions/common/url_pattern_set.h"
#include "extensions/renderer/dispatcher.h"
#include "extensions/renderer/native_handler.h"
#include "extensions/renderer/resource_bundle_source_map.h"
#include "extensions/renderer/script_context.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/web/WebDocument.h"
#include "third_party/WebKit/public/web/WebSecurityPolicy.h"
#include "third_party/WebKit/public/web/WebView.h"

#if defined(ENABLE_WEBRTC)
#include "chrome/renderer/extensions/cast_streaming_native_handler.h"
#endif

using extensions::NativeHandler;

ChromeExtensionsDispatcherDelegate::ChromeExtensionsDispatcherDelegate() {
}

ChromeExtensionsDispatcherDelegate::~ChromeExtensionsDispatcherDelegate() {
}

scoped_ptr<extensions::ScriptContext>
ChromeExtensionsDispatcherDelegate::CreateScriptContext(
    const v8::Handle<v8::Context>& v8_context,
    blink::WebFrame* frame,
    const extensions::Extension* extension,
    extensions::Feature::Context context_type,
    const extensions::Extension* effective_extension,
    extensions::Feature::Context effective_context_type) {
  return scoped_ptr<extensions::ScriptContext>(
      new extensions::ChromeV8Context(v8_context,
                                      frame,
                                      extension,
                                      context_type,
                                      effective_extension,
                                      effective_context_type));
}

void ChromeExtensionsDispatcherDelegate::InitOriginPermissions(
    const extensions::Extension* extension,
    bool is_extension_active) {
  // TODO(jstritar): We should try to remove this special case. Also, these
  // whitelist entries need to be updated when the kManagement permission
  // changes.
  if (is_extension_active &&
      extension->permissions_data()->HasAPIPermission(
          extensions::APIPermission::kManagement)) {
    blink::WebSecurityPolicy::addOriginAccessWhitelistEntry(
        extension->url(),
        blink::WebString::fromUTF8(content::kChromeUIScheme),
        blink::WebString::fromUTF8(chrome::kChromeUIExtensionIconHost),
        false);
  }
}

void ChromeExtensionsDispatcherDelegate::RegisterNativeHandlers(
    extensions::Dispatcher* dispatcher,
    extensions::ModuleSystem* module_system,
    extensions::ScriptContext* context) {
#if !defined(ENABLE_EXTENSIONS)
  return;
#endif
  module_system->RegisterNativeHandler(
      "app",
      scoped_ptr<NativeHandler>(
          new extensions::AppBindings(dispatcher, context)));
  module_system->RegisterNativeHandler(
      "sync_file_system",
      scoped_ptr<NativeHandler>(
          new extensions::SyncFileSystemCustomBindings(context)));
  module_system->RegisterNativeHandler(
      "enterprise_platform_keys_natives",
      scoped_ptr<NativeHandler>(
          new extensions::EnterprisePlatformKeysNatives(context)));
  module_system->RegisterNativeHandler(
      "file_browser_handler",
      scoped_ptr<NativeHandler>(
          new extensions::FileBrowserHandlerCustomBindings(context)));
  module_system->RegisterNativeHandler(
      "file_manager_private",
      scoped_ptr<NativeHandler>(
          new extensions::FileManagerPrivateCustomBindings(context)));
  module_system->RegisterNativeHandler(
      "notifications_private",
      scoped_ptr<NativeHandler>(
          new extensions::NotificationsNativeHandler(context)));
  module_system->RegisterNativeHandler(
      "mediaGalleries",
      scoped_ptr<NativeHandler>(
          new extensions::MediaGalleriesCustomBindings(context)));
  module_system->RegisterNativeHandler(
      "page_capture",
      scoped_ptr<NativeHandler>(
          new extensions::PageCaptureCustomBindings(context)));
  module_system->RegisterNativeHandler(
      "tabs",
      scoped_ptr<NativeHandler>(new extensions::TabsCustomBindings(context)));
  module_system->RegisterNativeHandler(
      "webstore",
      scoped_ptr<NativeHandler>(new extensions::WebstoreBindings(context)));
#if defined(ENABLE_WEBRTC)
  module_system->RegisterNativeHandler(
      "cast_streaming_natives",
      scoped_ptr<NativeHandler>(
          new extensions::CastStreamingNativeHandler(context)));
#endif
  module_system->RegisterNativeHandler(
      "automationInternal",
      scoped_ptr<NativeHandler>(
          new extensions::AutomationInternalCustomBindings(context)));
}

void ChromeExtensionsDispatcherDelegate::PopulateSourceMap(
    extensions::ResourceBundleSourceMap* source_map) {
  // Custom bindings.
  source_map->RegisterSource("app", IDR_APP_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("automation", IDR_AUTOMATION_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("automationEvent", IDR_AUTOMATION_EVENT_JS);
  source_map->RegisterSource("automationNode", IDR_AUTOMATION_NODE_JS);
  source_map->RegisterSource("browserAction",
                             IDR_BROWSER_ACTION_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("declarativeContent",
                             IDR_DECLARATIVE_CONTENT_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("desktopCapture",
                             IDR_DESKTOP_CAPTURE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("developerPrivate",
                             IDR_DEVELOPER_PRIVATE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("downloads", IDR_DOWNLOADS_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("enterprise.platformKeys",
                             IDR_ENTERPRISE_PLATFORM_KEYS_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("enterprise.platformKeys.internalAPI",
                             IDR_ENTERPRISE_PLATFORM_KEYS_INTERNAL_API_JS);
  source_map->RegisterSource("enterprise.platformKeys.Key",
                             IDR_ENTERPRISE_PLATFORM_KEYS_KEY_JS);
  source_map->RegisterSource("enterprise.platformKeys.KeyPair",
                             IDR_ENTERPRISE_PLATFORM_KEYS_KEY_PAIR_JS);
  source_map->RegisterSource("enterprise.platformKeys.SubtleCrypto",
                             IDR_ENTERPRISE_PLATFORM_KEYS_SUBTLE_CRYPTO_JS);
  source_map->RegisterSource("enterprise.platformKeys.Token",
                             IDR_ENTERPRISE_PLATFORM_KEYS_TOKEN_JS);
  source_map->RegisterSource("enterprise.platformKeys.utils",
                             IDR_ENTERPRISE_PLATFORM_KEYS_UTILS_JS);
  source_map->RegisterSource("feedbackPrivate",
                             IDR_FEEDBACK_PRIVATE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("fileBrowserHandler",
                             IDR_FILE_BROWSER_HANDLER_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("fileManagerPrivate",
                             IDR_FILE_MANAGER_PRIVATE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("fileSystem", IDR_FILE_SYSTEM_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("fileSystemProvider",
                             IDR_FILE_SYSTEM_PROVIDER_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("gcm", IDR_GCM_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("identity", IDR_IDENTITY_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("imageWriterPrivate",
                             IDR_IMAGE_WRITER_PRIVATE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("input.ime", IDR_INPUT_IME_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("logPrivate", IDR_LOG_PRIVATE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("mediaGalleries",
                             IDR_MEDIA_GALLERIES_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("notifications",
                             IDR_NOTIFICATIONS_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("omnibox", IDR_OMNIBOX_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("pageAction", IDR_PAGE_ACTION_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("pageCapture",
                             IDR_PAGE_CAPTURE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("syncFileSystem",
                             IDR_SYNC_FILE_SYSTEM_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("systemIndicator",
                             IDR_SYSTEM_INDICATOR_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("tabCapture", IDR_TAB_CAPTURE_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("tabs", IDR_TABS_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("tts", IDR_TTS_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("ttsEngine", IDR_TTS_ENGINE_CUSTOM_BINDINGS_JS);
#if defined(ENABLE_WEBRTC)
  source_map->RegisterSource("cast.streaming.rtpStream",
                             IDR_CAST_STREAMING_RTP_STREAM_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("cast.streaming.session",
                             IDR_CAST_STREAMING_SESSION_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource(
      "cast.streaming.udpTransport",
      IDR_CAST_STREAMING_UDP_TRANSPORT_CUSTOM_BINDINGS_JS);
#endif
  source_map->RegisterSource("webstore", IDR_WEBSTORE_CUSTOM_BINDINGS_JS);

  // Custom types sources.
  source_map->RegisterSource("ChromeSetting", IDR_CHROME_SETTING_JS);
  source_map->RegisterSource("ContentSetting", IDR_CONTENT_SETTING_JS);
  source_map->RegisterSource("ChromeDirectSetting",
                             IDR_CHROME_DIRECT_SETTING_JS);

  // Platform app sources that are not API-specific..
  source_map->RegisterSource("appView", IDR_APP_VIEW_JS);
  source_map->RegisterSource("fileEntryBindingUtil",
                             IDR_FILE_ENTRY_BINDING_UTIL_JS);
  source_map->RegisterSource("extensionOptions", IDR_EXTENSION_OPTIONS_JS);
  source_map->RegisterSource("extensionOptionsEvents",
                             IDR_EXTENSION_OPTIONS_EVENTS_JS);
  source_map->RegisterSource("tagWatcher", IDR_TAG_WATCHER_JS);
  source_map->RegisterSource("chromeWebViewInternal",
                             IDR_CHROME_WEB_VIEW_INTERNAL_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("chromeWebView", IDR_CHROME_WEB_VIEW_JS);
  source_map->RegisterSource("chromeWebViewExperimental",
                             IDR_CHROME_WEB_VIEW_EXPERIMENTAL_JS);
  source_map->RegisterSource("webViewRequest",
                             IDR_WEB_VIEW_REQUEST_CUSTOM_BINDINGS_JS);
  source_map->RegisterSource("denyAppView", IDR_APP_VIEW_DENY_JS);
  source_map->RegisterSource("injectAppTitlebar", IDR_INJECT_APP_TITLEBAR_JS);
}

void ChromeExtensionsDispatcherDelegate::RequireAdditionalModules(
    extensions::ScriptContext* context,
    bool is_within_platform_app) {
  extensions::ModuleSystem* module_system = context->module_system();
  extensions::Feature::Context context_type = context->context_type();

  // TODO(kalman, fsamuel): Eagerly calling Require on context startup is
  // expensive. It would be better if there were a light way of detecting when
  // a webview or appview is created and only then set up the infrastructure.
  if (context_type == extensions::Feature::BLESSED_EXTENSION_CONTEXT &&
      is_within_platform_app &&
      extensions::GetCurrentChannel() <= chrome::VersionInfo::CHANNEL_DEV &&
      CommandLine::ForCurrentProcess()->HasSwitch(
          extensions::switches::kEnableAppWindowControls)) {
    module_system->Require("windowControls");
  }

  // Note: setting up the WebView class here, not the chrome.webview API.
  // The API will be automatically set up when first used.
  if (context->GetAvailability("webViewInternal").is_available()) {
    module_system->Require("chromeWebView");
    if (context->GetAvailability("webViewExperimentalInternal")
            .is_available()) {
      module_system->Require("chromeWebViewExperimental");
    }
  }

  if (extensions::FeatureSwitch::app_view()->IsEnabled() &&
      context->GetAvailability("appViewEmbedderInternal").is_available()) {
    module_system->Require("appView");
  } else if (context_type == extensions::Feature::BLESSED_EXTENSION_CONTEXT) {
    module_system->Require("denyAppView");
  }

  if (extensions::FeatureSwitch::embedded_extension_options()->IsEnabled() &&
      context->GetAvailability("extensionOptionsInternal").is_available()) {
    module_system->Require("extensionOptions");
  }
}

void ChromeExtensionsDispatcherDelegate::OnActiveExtensionsUpdated(
    const std::set<std::string>& extension_ids) {
  // In single-process mode, the browser process reports the active extensions.
  if (CommandLine::ForCurrentProcess()->HasSwitch(::switches::kSingleProcess))
    return;
  crash_keys::SetActiveExtensions(extension_ids);
}

void ChromeExtensionsDispatcherDelegate::SetChannel(int channel) {
  extensions::SetCurrentChannel(
      static_cast<chrome::VersionInfo::Channel>(channel));
}

void ChromeExtensionsDispatcherDelegate::ClearTabSpecificPermissions(
    const extensions::Dispatcher* dispatcher,
    int tab_id,
    const std::vector<std::string>& extension_ids) {
  for (std::vector<std::string>::const_iterator it = extension_ids.begin();
       it != extension_ids.end();
       ++it) {
    const extensions::Extension* extension =
        dispatcher->extensions()->GetByID(*it);
    if (extension)
      extension->permissions_data()->ClearTabSpecificPermissions(tab_id);
  }
}

void ChromeExtensionsDispatcherDelegate::UpdateTabSpecificPermissions(
    const extensions::Dispatcher* dispatcher,
    const GURL& url,
    int tab_id,
    const std::string& extension_id,
    const extensions::URLPatternSet& origin_set) {
  content::RenderView* view = extensions::TabFinder::Find(tab_id);

  // For now, the message should only be sent to the render view that contains
  // the target tab. This may change. Either way, if this is the target tab it
  // gives us the chance to check against the URL to avoid races.
  DCHECK(view);
  GURL active_url(view->GetWebView()->mainFrame()->document().url());
  if (active_url != url)
    return;

  const extensions::Extension* extension =
      dispatcher->extensions()->GetByID(extension_id);
  if (!extension)
    return;

  extension->permissions_data()->UpdateTabSpecificPermissions(
      tab_id,
      new extensions::PermissionSet(extensions::APIPermissionSet(),
                                    extensions::ManifestPermissionSet(),
                                    origin_set,
                                    extensions::URLPatternSet()));
}
