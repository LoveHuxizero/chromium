// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/cast_channel/cast_channel_api.h"

#include "base/json/json_writer.h"
#include "base/memory/scoped_ptr.h"
#include "base/values.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/extensions/api/cast_channel/cast_socket.h"
#include "chrome/browser/net/chrome_net_log.h"
#include "content/public/browser/browser_thread.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_system.h"
#include "net/base/net_errors.h"
#include "url/gurl.h"

namespace extensions {

namespace Close = cast_channel::Close;
namespace OnError = cast_channel::OnError;
namespace OnMessage = cast_channel::OnMessage;
namespace Open = cast_channel::Open;
namespace Send = cast_channel::Send;
using cast_channel::CastSocket;
using cast_channel::ChannelError;
using cast_channel::ChannelInfo;
using cast_channel::MessageInfo;
using cast_channel::ReadyState;
using content::BrowserThread;

namespace {

// T is an extension dictionary (MessageInfo or ChannelInfo)
template <class T>
std::string ParamToString(const T& info) {
  scoped_ptr<base::DictionaryValue> dict = info.ToValue();
  std::string out;
  base::JSONWriter::Write(dict.get(), &out);
  return out;
}

}  // namespace

CastChannelAPI::CastChannelAPI(content::BrowserContext* context)
    : browser_context_(context) {
  DCHECK(browser_context_);
}

// static
CastChannelAPI* CastChannelAPI::Get(content::BrowserContext* context) {
  return BrowserContextKeyedAPIFactory<CastChannelAPI>::Get(context);
}

static base::LazyInstance<BrowserContextKeyedAPIFactory<CastChannelAPI> >
    g_factory = LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<CastChannelAPI>*
CastChannelAPI::GetFactoryInstance() {
  return g_factory.Pointer();
}

scoped_ptr<CastSocket> CastChannelAPI::CreateCastSocket(
    const std::string& extension_id, const GURL& url) {
  if (socket_for_test_.get()) {
    return socket_for_test_.Pass();
  } else {
    return scoped_ptr<CastSocket>(
        new CastSocket(extension_id, url, this,
                       g_browser_process->net_log()));
  }
}

void CastChannelAPI::SetSocketForTest(scoped_ptr<CastSocket> socket_for_test) {
  socket_for_test_ = socket_for_test.Pass();
}

void CastChannelAPI::OnError(const CastSocket* socket,
                             cast_channel::ChannelError error) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  ChannelInfo channel_info;
  socket->FillChannelInfo(&channel_info);
  channel_info.error_state = error;
  scoped_ptr<base::ListValue> results = OnError::Create(channel_info);
  scoped_ptr<Event> event(new Event(OnError::kEventName, results.Pass()));
  extensions::ExtensionSystem::Get(browser_context_)
      ->event_router()
      ->DispatchEventToExtension(socket->owner_extension_id(), event.Pass());
}

void CastChannelAPI::OnMessage(const CastSocket* socket,
                               const MessageInfo& message_info) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  ChannelInfo channel_info;
  socket->FillChannelInfo(&channel_info);
  scoped_ptr<base::ListValue> results =
    OnMessage::Create(channel_info, message_info);
  VLOG(1) << "Sending message " << ParamToString(message_info)
          << " to channel " << ParamToString(channel_info);
  scoped_ptr<Event> event(new Event(OnMessage::kEventName, results.Pass()));
  extensions::ExtensionSystem::Get(browser_context_)
      ->event_router()
      ->DispatchEventToExtension(socket->owner_extension_id(), event.Pass());
}

CastChannelAPI::~CastChannelAPI() {}

CastChannelAsyncApiFunction::CastChannelAsyncApiFunction()
  : manager_(NULL), error_(cast_channel::CHANNEL_ERROR_NONE) { }

CastChannelAsyncApiFunction::~CastChannelAsyncApiFunction() { }

bool CastChannelAsyncApiFunction::PrePrepare() {
  manager_ = ApiResourceManager<CastSocket>::Get(browser_context());
  return true;
}

bool CastChannelAsyncApiFunction::Respond() {
  return error_ != cast_channel::CHANNEL_ERROR_NONE;
}

CastSocket* CastChannelAsyncApiFunction::GetSocketOrCompleteWithError(
    int channel_id) {
  CastSocket* socket = GetSocket(channel_id);
  if (!socket) {
    SetResultFromError(cast_channel::CHANNEL_ERROR_INVALID_CHANNEL_ID);
    AsyncWorkCompleted();
  }
  return socket;
}

int CastChannelAsyncApiFunction::AddSocket(CastSocket* socket) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  DCHECK(socket);
  DCHECK(manager_);
  const int id = manager_->Add(socket);
  socket->set_id(id);
  return id;
}

void CastChannelAsyncApiFunction::RemoveSocket(int channel_id) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  DCHECK(manager_);
  manager_->Remove(extension_->id(), channel_id);
}

void CastChannelAsyncApiFunction::SetResultFromSocket(int channel_id) {
  CastSocket* socket = GetSocket(channel_id);
  DCHECK(socket);
  ChannelInfo channel_info;
  socket->FillChannelInfo(&channel_info);
  error_ = socket->error_state();
  SetResultFromChannelInfo(channel_info);
}

void CastChannelAsyncApiFunction::SetResultFromError(ChannelError error) {
  ChannelInfo channel_info;
  channel_info.channel_id = -1;
  channel_info.url = "";
  channel_info.ready_state = cast_channel::READY_STATE_CLOSED;
  channel_info.error_state = error;
  SetResultFromChannelInfo(channel_info);
  error_ = error;
}

CastSocket* CastChannelAsyncApiFunction::GetSocket(int channel_id) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  DCHECK(manager_);
  return manager_->Get(extension_->id(), channel_id);
}

void CastChannelAsyncApiFunction::SetResultFromChannelInfo(
    const ChannelInfo& channel_info) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  SetResult(channel_info.ToValue().release());
}

CastChannelOpenFunction::CastChannelOpenFunction()
  : new_channel_id_(0) { }

CastChannelOpenFunction::~CastChannelOpenFunction() { }

bool CastChannelOpenFunction::PrePrepare() {
  api_ = CastChannelAPI::Get(browser_context());
  return CastChannelAsyncApiFunction::PrePrepare();
}

bool CastChannelOpenFunction::Prepare() {
  params_ = Open::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void CastChannelOpenFunction::AsyncWorkStart() {
  DCHECK(api_);
  scoped_ptr<CastSocket> socket = api_->CreateCastSocket(extension_->id(),
                                                         GURL(params_->url));
  new_channel_id_ = AddSocket(socket.release());
  GetSocket(new_channel_id_)->Connect(
      base::Bind(&CastChannelOpenFunction::OnOpen, this));
}

void CastChannelOpenFunction::OnOpen(int result) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  SetResultFromSocket(new_channel_id_);
  AsyncWorkCompleted();
}

CastChannelSendFunction::CastChannelSendFunction() { }

CastChannelSendFunction::~CastChannelSendFunction() { }

bool CastChannelSendFunction::Prepare() {
  params_ = Send::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void CastChannelSendFunction::AsyncWorkStart() {
  CastSocket* socket = GetSocketOrCompleteWithError(
      params_->channel.channel_id);
  if (socket)
    socket->SendMessage(params_->message,
                        base::Bind(&CastChannelSendFunction::OnSend, this));
}

void CastChannelSendFunction::OnSend(int result) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  if (result < 0) {
    SetResultFromError(cast_channel::CHANNEL_ERROR_SOCKET_ERROR);
  } else {
    SetResultFromSocket(params_->channel.channel_id);
  }
  AsyncWorkCompleted();
}

CastChannelCloseFunction::CastChannelCloseFunction() { }

CastChannelCloseFunction::~CastChannelCloseFunction() { }

bool CastChannelCloseFunction::Prepare() {
  params_ = Close::Params::Create(*args_);
  EXTENSION_FUNCTION_VALIDATE(params_.get());
  return true;
}

void CastChannelCloseFunction::AsyncWorkStart() {
  CastSocket* socket = GetSocketOrCompleteWithError(
      params_->channel.channel_id);
  if (socket)
    socket->Close(base::Bind(&CastChannelCloseFunction::OnClose, this));
}

void CastChannelCloseFunction::OnClose(int result) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  VLOG(1) << "CastChannelCloseFunction::OnClose result = " << result;
  if (result < 0) {
    SetResultFromError(cast_channel::CHANNEL_ERROR_SOCKET_ERROR);
  } else {
    int channel_id = params_->channel.channel_id;
    SetResultFromSocket(channel_id);
    RemoveSocket(channel_id);
  }
  AsyncWorkCompleted();
}

}  // namespace extensions
