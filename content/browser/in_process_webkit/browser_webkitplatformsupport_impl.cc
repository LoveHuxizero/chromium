// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/in_process_webkit/browser_webkitplatformsupport_impl.h"

#include "base/file_util.h"
#include "base/logging.h"
#include "base/sys_info.h"
#include "content/browser/gpu/browser_gpu_channel_host_factory.h"
#include "third_party/WebKit/public/platform/WebString.h"

namespace content {

BrowserWebKitPlatformSupportImpl::BrowserWebKitPlatformSupportImpl() {
  file_utilities_.set_sandbox_enabled(false);
}

BrowserWebKitPlatformSupportImpl::~BrowserWebKitPlatformSupportImpl() {
}

WebKit::WebClipboard* BrowserWebKitPlatformSupportImpl::clipboard() {
  NOTREACHED();
  return NULL;
}

WebKit::WebMimeRegistry* BrowserWebKitPlatformSupportImpl::mimeRegistry() {
  NOTREACHED();
  return NULL;
}

WebKit::WebFileUtilities* BrowserWebKitPlatformSupportImpl::fileUtilities() {
  return &file_utilities_;
}

WebKit::WebSandboxSupport* BrowserWebKitPlatformSupportImpl::sandboxSupport() {
  NOTREACHED();
  return NULL;
}

bool BrowserWebKitPlatformSupportImpl::sandboxEnabled() {
  return false;
}

unsigned long long BrowserWebKitPlatformSupportImpl::visitedLinkHash(
    const char* canonical_url,
    size_t length) {
  NOTREACHED();
  return 0;
}

bool BrowserWebKitPlatformSupportImpl::isLinkVisited(
    unsigned long long link_hash) {
  NOTREACHED();
  return false;
}

WebKit::WebMessagePortChannel*
BrowserWebKitPlatformSupportImpl::createMessagePortChannel() {
  NOTREACHED();
  return NULL;
}

void BrowserWebKitPlatformSupportImpl::prefetchHostName(
    const WebKit::WebString&) {
  NOTREACHED();
}

WebKit::WebString BrowserWebKitPlatformSupportImpl::defaultLocale() {
  NOTREACHED();
  return WebKit::WebString();
}

WebKit::WebThemeEngine* BrowserWebKitPlatformSupportImpl::themeEngine() {
  NOTREACHED();
  return NULL;
}

WebKit::WebURLLoader* BrowserWebKitPlatformSupportImpl::createURLLoader() {
  NOTREACHED();
  return NULL;
}

WebKit::WebSocketStreamHandle*
BrowserWebKitPlatformSupportImpl::createSocketStreamHandle() {
  NOTREACHED();
  return NULL;
}

void BrowserWebKitPlatformSupportImpl::getPluginList(bool refresh,
    WebKit::WebPluginListBuilder* builder) {
  NOTREACHED();
}

int BrowserWebKitPlatformSupportImpl::databaseDeleteFile(
    const WebKit::WebString& vfs_file_name, bool sync_dir) {
  const base::FilePath& path = base::FilePath::FromUTF16Unsafe(vfs_file_name);
  return base::Delete(path, false) ? 0 : 1;
}

long long BrowserWebKitPlatformSupportImpl::availableDiskSpaceInBytes(
    const WebKit::WebString& fileName) {
  return base::SysInfo::AmountOfFreeDiskSpace(
      base::FilePath::FromUTF16Unsafe(fileName));
}

}  // namespace content
