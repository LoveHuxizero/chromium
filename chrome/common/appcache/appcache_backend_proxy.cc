// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/appcache/appcache_backend_proxy.h"

#include "chrome/common/render_messages.h"

void AppCacheBackendProxy::RegisterHost(int host_id) {
  sender_->Send(new AppCacheMsg_RegisterHost(host_id));
}

void AppCacheBackendProxy::UnregisterHost(int host_id) {
  sender_->Send(new AppCacheMsg_UnregisterHost(host_id));
}

void AppCacheBackendProxy::SelectCache(
    int host_id,
    const GURL& document_url,
    const int64 cache_document_was_loaded_from,
    const GURL& manifest_url) {
  sender_->Send(new AppCacheMsg_SelectCache(
                                    host_id, document_url,
                                    cache_document_was_loaded_from,
                                    manifest_url));
}

void AppCacheBackendProxy::SelectCacheForWorker(
    int host_id, int parent_process_id, int parent_host_id) {
  sender_->Send(new AppCacheMsg_SelectCacheForWorker(
                                    host_id, parent_process_id,
                                    parent_host_id));
}

void AppCacheBackendProxy::SelectCacheForSharedWorker(
    int host_id, int64 appcache_id) {
  sender_->Send(new AppCacheMsg_SelectCacheForSharedWorker(
                                    host_id, appcache_id));
}

void AppCacheBackendProxy::MarkAsForeignEntry(
    int host_id, const GURL& document_url,
    int64 cache_document_was_loaded_from) {
  sender_->Send(new AppCacheMsg_MarkAsForeignEntry(
                                    host_id, document_url,
                                    cache_document_was_loaded_from));
}

appcache::Status AppCacheBackendProxy::GetStatus(int host_id) {
  appcache::Status status = appcache::UNCACHED;
  sender_->Send(new AppCacheMsg_GetStatus(host_id, &status));
  return status;
}

bool AppCacheBackendProxy::StartUpdate(int host_id) {
  bool result = false;
  sender_->Send(new AppCacheMsg_StartUpdate(host_id, &result));
  return result;
}

bool AppCacheBackendProxy::SwapCache(int host_id) {
  bool result = false;
  sender_->Send(new AppCacheMsg_SwapCache(host_id, &result));
  return result;
}
