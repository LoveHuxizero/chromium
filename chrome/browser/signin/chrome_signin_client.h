// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SIGNIN_CHROME_SIGNIN_CLIENT_H_
#define CHROME_BROWSER_SIGNIN_CHROME_SIGNIN_CLIENT_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "components/keyed_service/core/keyed_service.h"
#include "components/signin/core/browser/signin_client.h"
#include "content/public/browser/render_process_host_observer.h"

class CookieSettings;
class Profile;

class ChromeSigninClient : public SigninClient,
                           public KeyedService,
                           public content::RenderProcessHostObserver {
 public:
  explicit ChromeSigninClient(Profile* profile);
  virtual ~ChromeSigninClient();

  // Utility methods.
  static bool ProfileAllowsSigninCookies(Profile* profile);
  static bool SettingsAllowSigninCookies(CookieSettings* cookie_settings);

  // Tracks the privileged signin process identified by |host_id| so that we
  // can later ask (via IsSigninProcess) if it is safe to sign the user in from
  // the current context (see OneClickSigninHelper).  All of this tracking
  // state is reset once the renderer process terminates.
  //
  // N.B. This is the id returned by RenderProcessHost::GetID().
  // TODO(guohui): Eliminate these APIs once the web-based signin flow is
  // replaced by a native flow. crbug.com/347247
  void SetSigninProcess(int host_id);
  void ClearSigninProcess();
  bool IsSigninProcess(int host_id) const;
  bool HasSigninProcess() const;

  // content::RenderProcessHostObserver implementation.
  virtual void RenderProcessHostDestroyed(content::RenderProcessHost* host)
      OVERRIDE;

  // SigninClient implementation.
  virtual PrefService* GetPrefs() OVERRIDE;
  virtual scoped_refptr<TokenWebData> GetDatabase() OVERRIDE;
  virtual bool CanRevokeCredentials() OVERRIDE;
  virtual net::URLRequestContextGetter* GetURLRequestContext() OVERRIDE;
  virtual bool ShouldMergeSigninCredentialsIntoCookieJar() OVERRIDE;

  // Returns a string describing the chrome version environment. Version format:
  // <Build Info> <OS> <Version number> (<Last change>)<channel or "-devel">
  // If version information is unavailable, returns "invalid."
  virtual std::string GetProductVersion() OVERRIDE;
  virtual void GoogleSigninSucceeded(const std::string& username,
                                     const std::string& password) OVERRIDE;

 private:
  Profile* profile_;

  // See SetSigninProcess. Tracks the currently active signin process
  // by ID, if there is one.
  int signin_host_id_;

  // The RenderProcessHosts being observed.
  std::set<content::RenderProcessHost*> signin_hosts_observed_;

  DISALLOW_COPY_AND_ASSIGN(ChromeSigninClient);
};

#endif  // CHROME_BROWSER_SIGNIN_CHROME_SIGNIN_CLIENT_H_
