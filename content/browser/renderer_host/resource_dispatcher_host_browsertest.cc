// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/string_util.h"
#include "base/synchronization/waitable_event.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/browser/download/download_manager_impl.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/common/test_url_constants.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/web_contents.h"
#include "content/public/common/url_constants.h"
#include "content/test/net/url_request_failed_job.h"
#include "content/test/net/url_request_mock_http_job.h"
#include "net/base/net_errors.h"
#include "net/cookies/cookie_store.h"
#include "net/test/test_server.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"

using content::BrowserContext;
using content::BrowserThread;
using content::DownloadManager;

namespace {

void GetCookiesCallback(std::string* cookies_out,
                        base::WaitableEvent* event,
                        const std::string& cookies) {
  *cookies_out = cookies;
  event->Signal();
}

void GetCookiesOnIOThread(const GURL& url,
                          net::URLRequestContextGetter* context_getter,
                          base::WaitableEvent* event,
                          std::string* cookies) {
  net::CookieStore* cookie_store =
      context_getter->GetURLRequestContext()->cookie_store();
  cookie_store->GetCookiesWithOptionsAsync(
      url, net::CookieOptions(),
      base::Bind(&GetCookiesCallback,
                 base::Unretained(cookies), base::Unretained(event)));
}

}  // namespace

class ResourceDispatcherHostBrowserTest : public InProcessBrowserTest,
                                          public DownloadManager::Observer {
 public:
  ResourceDispatcherHostBrowserTest() : got_downloads_(false) {
    EnableDOMAutomation();
  }

 protected:
  virtual void SetUpOnMainThread() OVERRIDE {
    FilePath path = ui_test_utils::GetTestFilePath(FilePath(), FilePath());
    BrowserThread::PostTask(
        BrowserThread::IO, FROM_HERE,
        base::Bind(&URLRequestMockHTTPJob::AddUrlHandler, path));
    BrowserThread::PostTask(
        BrowserThread::IO, FROM_HERE,
        base::Bind(&URLRequestFailedJob::AddUrlHandler));
  }

  virtual void ModelChanged(DownloadManager* manager) OVERRIDE {
    if (!got_downloads_)
      got_downloads_ = !!manager->InProgressCount();
  }

  content::RenderViewHost* render_view_host() {
    return browser()->GetActiveWebContents()->GetRenderViewHost();
  }

  GURL GetMockURL(const std::string& file) {
    return URLRequestMockHTTPJob::GetMockUrl(FilePath().AppendASCII(file));
  }

  void CheckTitleTest(const GURL& url,
                      const std::string& expected_title,
                      int expected_navigations) {
    string16 expected_title16(ASCIIToUTF16(expected_title));
    ui_test_utils::TitleWatcher title_watcher(
        browser()->GetActiveWebContents(), expected_title16);
    ui_test_utils::NavigateToURLBlockUntilNavigationsComplete(
        browser(), url, expected_navigations);
    EXPECT_EQ(expected_title16, title_watcher.WaitAndGetTitle());
  }

  bool GetPopupTitle(const GURL& url, string16* title);

  std::string GetCookies(const GURL& url) {
    std::string cookies;
    base::WaitableEvent event(true, false);
    net::URLRequestContextGetter* context_getter =
        GetBrowserContext()->GetRequestContext();

    BrowserThread::PostTask(
        BrowserThread::IO, FROM_HERE,
        base::Bind(&GetCookiesOnIOThread, url,
                   make_scoped_refptr(context_getter), &event, &cookies));
    event.Wait();
    return cookies;
  }

  bool got_downloads() const { return got_downloads_; }

 private:
  bool got_downloads_;
};

bool ResourceDispatcherHostBrowserTest::GetPopupTitle(const GURL& url,
                                                      string16* title) {
  ui_test_utils::NavigateToURL(browser(), url);

  ui_test_utils::WindowedTabAddedNotificationObserver observer(
      content::NotificationService::AllSources());

  // Create dynamic popup.
  if (!ui_test_utils::ExecuteJavaScript(
          render_view_host(), L"", L"OpenPopup();"))
    return false;

  observer.Wait();

  std::set<Browser*> excluded;
  excluded.insert(browser());
  Browser* popup = ui_test_utils::GetBrowserNotInSet(excluded);
  if (!popup)
    return false;

  *title = popup->GetWindowTitleForCurrentTab();
  return true;
}

// Test title for content created by javascript window.open().
// See http://crbug.com/5988
IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest, DynamicTitle1) {
  ASSERT_TRUE(test_server()->Start());

  GURL url(test_server()->GetURL("files/dynamic1.html"));
  string16 title;
  ASSERT_TRUE(GetPopupTitle(url, &title));
  EXPECT_TRUE(StartsWith(title, ASCIIToUTF16("My Popup Title"), true))
      << "Actual title: " << title;
}

// Test title for content created by javascript window.open().
// See http://crbug.com/5988
IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest, DynamicTitle2) {
  ASSERT_TRUE(test_server()->Start());

  GURL url(test_server()->GetURL("files/dynamic2.html"));
  string16 title;
  ASSERT_TRUE(GetPopupTitle(url, &title));
  EXPECT_TRUE(StartsWith(title, ASCIIToUTF16("My Dynamic Title"), true))
      << "Actual title: " << title;
}

IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       SniffHTMLWithNoContentType) {
  CheckTitleTest(GetMockURL("content-sniffer-test0.html"),
                 "Content Sniffer Test 0", 1);
}

IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       RespectNoSniffDirective) {
  CheckTitleTest(GetMockURL("nosniff-test.html"),
                 "mock.http/nosniff-test.html", 1);
}

IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       DoNotSniffHTMLFromTextPlain) {
  CheckTitleTest(GetMockURL("content-sniffer-test1.html"),
                 "mock.http/content-sniffer-test1.html", 1);
}

IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       DoNotSniffHTMLFromImageGIF) {
  CheckTitleTest(GetMockURL("content-sniffer-test2.html"),
                 "mock.http/content-sniffer-test2.html", 1);
}

IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       SniffNoContentTypeNoData) {
  // Make sure no downloads start.
  BrowserContext::GetDownloadManager(GetBrowserContext())->AddObserver(this);
  CheckTitleTest(GetMockURL("content-sniffer-test3.html"),
                 "Content Sniffer Test 3", 1);
  EXPECT_EQ(1, browser()->tab_count());
  ASSERT_FALSE(got_downloads());
}

IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       ContentDispositionEmpty) {
  CheckTitleTest(GetMockURL("content-disposition-empty.html"), "success", 1);
}

IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       ContentDispositionInline) {
  CheckTitleTest(GetMockURL("content-disposition-inline.html"), "success", 1);
}

// Test for bug #1091358.
IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest, SyncXMLHttpRequest) {
  ASSERT_TRUE(test_server()->Start());
  ui_test_utils::NavigateToURL(
      browser(), test_server()->GetURL("files/sync_xmlhttprequest.html"));

  // Let's check the XMLHttpRequest ran successfully.
  bool success = false;
  EXPECT_TRUE(ui_test_utils::ExecuteJavaScriptAndExtractBool(
      browser()->GetActiveWebContents()->GetRenderViewHost(),
      L"",
      L"window.domAutomationController.send(DidSyncRequestSucceed());",
      &success));
  EXPECT_TRUE(success);
}

// If this flakes, use http://crbug.com/62776.
IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       SyncXMLHttpRequest_Disallowed) {
  ASSERT_TRUE(test_server()->Start());
  ui_test_utils::NavigateToURL(
      browser(),
      test_server()->GetURL("files/sync_xmlhttprequest_disallowed.html"));

  // Let's check the XMLHttpRequest ran successfully.
  bool success = false;
  EXPECT_TRUE(ui_test_utils::ExecuteJavaScriptAndExtractBool(
      browser()->GetActiveWebContents()->GetRenderViewHost(),
      L"",
      L"window.domAutomationController.send(DidSucceed());",
      &success));
  EXPECT_TRUE(success);
}

// Test for bug #1159553 -- A synchronous xhr (whose content-type is
// downloadable) would trigger download and hang the renderer process,
// if executed while navigating to a new page.
// If this flakes, use http://crbug.com/56264.
IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       SyncXMLHttpRequest_DuringUnload) {
  ASSERT_TRUE(test_server()->Start());
  BrowserContext::GetDownloadManager(GetBrowserContext())->AddObserver(this);

  CheckTitleTest(
      test_server()->GetURL("files/sync_xmlhttprequest_during_unload.html"),
      "sync xhr on unload", 1);

  // Navigate to a new page, to dispatch unload event and trigger xhr.
  // (the bug would make this step hang the renderer).
  CheckTitleTest(
      test_server()->GetURL("files/title2.html"),
      "Title Of Awesomeness", 1);

  ASSERT_FALSE(got_downloads());
}

// Tests that onunload is run for cross-site requests.  (Bug 1114994)
IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       CrossSiteOnunloadCookie) {
  ASSERT_TRUE(test_server()->Start());

  GURL url = test_server()->GetURL("files/onunload_cookie.html");
  CheckTitleTest(url, "set cookie on unload", 1);

  // Navigate to a new cross-site page, to dispatch unload event and set the
  // cookie.
  CheckTitleTest(GetMockURL("content-sniffer-test0.html"),
                 "Content Sniffer Test 0", 1);

  // Check that the cookie was set.
  EXPECT_EQ("onunloadCookie=foo", GetCookies(url));
}

// ResourceDispatcherHostBrowserTest.CrossSiteImmediateLoadOnunloadCookie is
// flaky on Windows. http://crbug.com/130404
#if defined (OS_WIN)
#define MAYBE_CrossSiteImmediateLoadOnunloadCookie \
    DISABLED_CrossSiteImmediateLoadOnunloadCookie
#else
#define MAYBE_CrossSiteImmediateLoadOnunloadCookie \
    CrossSiteImmediateLoadOnunloadCookie
#endif

// Tests that onunload is run for cross-site requests to URLs that complete
// without network loads (e.g., about:blank, data URLs).
IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       MAYBE_CrossSiteImmediateLoadOnunloadCookie) {
  ASSERT_TRUE(test_server()->Start());

  GURL url = test_server()->GetURL("files/onunload_cookie.html");
  CheckTitleTest(url, "set cookie on unload", 1);

  // Navigate to a cross-site page that loads immediately without making a
  // network request.  The unload event should still be run.
  ui_test_utils::NavigateToURL(browser(), GURL("about:blank"));

  // Check that the cookie was set.
  EXPECT_EQ("onunloadCookie=foo", GetCookies(url));
}

// Tests that the unload handler is not run for 204 responses.
// If this flakes use http://crbug.com/80596.
IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       CrossSiteNoUnloadOn204) {
  ASSERT_TRUE(test_server()->Start());

  // Start with a URL that sets a cookie in its unload handler.
  GURL url = test_server()->GetURL("files/onunload_cookie.html");
  CheckTitleTest(url, "set cookie on unload", 1);

  // Navigate to a cross-site URL that returns a 204 No Content response.
  ui_test_utils::NavigateToURL(browser(), test_server()->GetURL("nocontent"));

  // Check that the unload cookie was not set.
  EXPECT_EQ("", GetCookies(url));
}

#if !defined(OS_MACOSX)
// Tests that the onbeforeunload and onunload logic is short-circuited if the
// old renderer is gone.  In that case, we don't want to wait for the old
// renderer to run the handlers.
// We need to disable this on Mac because the crash causes the OS CrashReporter
// process to kick in to analyze the poor dead renderer.  Unfortunately, if the
// app isn't stripped of debug symbols, this takes about five minutes to
// complete and isn't conducive to quick turnarounds. As we don't currently
// strip the app on the build bots, this is bad times.
IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest, CrossSiteAfterCrash) {
  // Cause the renderer to crash.
  ui_test_utils::WindowedNotificationObserver crash_observer(
        content::NOTIFICATION_RENDERER_PROCESS_CLOSED,
        content::NotificationService::AllSources());
  ui_test_utils::NavigateToURL(browser(), GURL(chrome::kChromeUICrashURL));
  // Wait for browser to notice the renderer crash.
  crash_observer.Wait();

  // Navigate to a new cross-site page.  The browser should not wait around for
  // the old renderer's on{before}unload handlers to run.
  CheckTitleTest(GetMockURL("content-sniffer-test0.html"),
                 "Content Sniffer Test 0", 1);
}
#endif  // !defined(OS_MACOSX)

// Tests that cross-site navigations work when the new page does not go through
// the BufferedEventHandler (e.g., non-http{s} URLs).  (Bug 1225872)
IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       CrossSiteNavigationNonBuffered) {
  // Start with an HTTP page.
  CheckTitleTest(GetMockURL("content-sniffer-test0.html"),
                 "Content Sniffer Test 0", 1);

  // Now load a file:// page, which does not use the BufferedEventHandler.
  // Make sure that the page loads and displays a title, and doesn't get stuck.
  GURL url = ui_test_utils::GetTestUrl(FilePath(),
                                       FilePath().AppendASCII("title2.html"));
  CheckTitleTest(url, "Title Of Awesomeness", 1);
}

// Tests that a cross-site navigation to an error page (resulting in the link
// doctor page) still runs the onunload handler and can support navigations
// away from the link doctor page.  (Bug 1235537)
// Flaky: http://crbug.com/100823
IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       CrossSiteNavigationErrorPage) {
  ASSERT_TRUE(test_server()->Start());

  GURL url(test_server()->GetURL("files/onunload_cookie.html"));
  CheckTitleTest(url, "set cookie on unload", 1);

  // Navigate to a new cross-site URL that results in an error page.
  // TODO(creis): If this causes crashes or hangs, it might be for the same
  // reason as ErrorPageTest::DNSError.  See bug 1199491 and
  // http://crbug.com/22877.
  GURL failed_url = URLRequestFailedJob::GetMockHttpUrl(
      net::ERR_NAME_NOT_RESOLVED);
  ui_test_utils::NavigateToURLBlockUntilNavigationsComplete(
      browser(), failed_url, 2);

  EXPECT_NE(ASCIIToUTF16("set cookie on unload"),
            browser()->GetActiveWebContents()->GetTitle());

  // Check that the cookie was set, meaning that the onunload handler ran.
  EXPECT_EQ("onunloadCookie=foo", GetCookies(url));

  // Check that renderer-initiated navigations still work.  In a previous bug,
  // the ResourceDispatcherHost would think that such navigations were
  // cross-site, because we didn't clean up from the previous request.  Since
  // WebContentsImpl was in the NORMAL state, it would ignore the attempt to run
  // the onunload handler, and the navigation would fail. We can't test by
  // redirecting to javascript:window.location='someURL', since javascript:
  // URLs are prohibited by policy from interacting with sensitive chrome
  // pages of which the error page is one.  Instead, use automation to kick
  // off the navigation, and wait to see that the tab loads.
  string16 expected_title16(ASCIIToUTF16("Title Of Awesomeness"));
  ui_test_utils::TitleWatcher title_watcher(
      browser()->GetActiveWebContents(), expected_title16);

  bool success;
  GURL test_url(test_server()->GetURL("files/title2.html"));
  std::string redirect_script = "window.location='" +
      test_url.possibly_invalid_spec() + "';" +
      "window.domAutomationController.send(true);";
  EXPECT_TRUE(ui_test_utils::ExecuteJavaScriptAndExtractBool(
      browser()->GetActiveWebContents()->GetRenderViewHost(),
      L"", ASCIIToWide(redirect_script), &success));
  EXPECT_EQ(expected_title16, title_watcher.WaitAndGetTitle());
}

IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       CrossSiteNavigationErrorPage2) {
  ASSERT_TRUE(test_server()->Start());

  GURL url(test_server()->GetURL("files/title2.html"));
  CheckTitleTest(url, "Title Of Awesomeness", 1);

  // Navigate to a new cross-site URL that results in an error page.
  // TODO(creis): If this causes crashes or hangs, it might be for the same
  // reason as ErrorPageTest::DNSError.  See bug 1199491 and
  // http://crbug.com/22877.
  GURL failed_url = URLRequestFailedJob::GetMockHttpUrl(
      net::ERR_NAME_NOT_RESOLVED);

  ui_test_utils::NavigateToURLBlockUntilNavigationsComplete(
      browser(), failed_url, 2);
  EXPECT_NE(ASCIIToUTF16("Title Of Awesomeness"),
            browser()->GetActiveWebContents()->GetTitle());

  // Repeat navigation.  We are testing that this completes.
  ui_test_utils::NavigateToURLBlockUntilNavigationsComplete(
      browser(), failed_url, 2);
  EXPECT_NE(ASCIIToUTF16("Title Of Awesomeness"),
            browser()->GetActiveWebContents()->GetTitle());
}

IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       CrossOriginRedirectBlocked) {
  // We expect the following URL requests from this test:
  // 1-  http://mock.http/cross-origin-redirect-blocked.html
  // 2-  http://mock.http/redirect-to-title2.html
  // 3-  http://mock.http/title2.html
  //
  // If the redirect in #2 were not blocked, we'd also see a request
  // for http://mock.http:4000/title2.html, and the title would be different.
  CheckTitleTest(GetMockURL("cross-origin-redirect-blocked.html"),
                 "Title Of More Awesomeness", 2);
}

// Tests that ResourceRequestInfoImpl is updated correctly on failed
// requests, to prevent calling Read on a request that has already failed.
// See bug 40250.
IN_PROC_BROWSER_TEST_F(ResourceDispatcherHostBrowserTest,
                       CrossSiteFailedRequest) {
  // Visit another URL first to trigger a cross-site navigation.
  GURL url(chrome::kTestNewTabURL);
  ui_test_utils::NavigateToURL(browser(), url);

  // Visit a URL that fails without calling ResourceDispatcherHost::Read.
  GURL broken_url("chrome://theme");
  CheckTitleTest(broken_url, "chrome://theme/ is not available", 1);
}
