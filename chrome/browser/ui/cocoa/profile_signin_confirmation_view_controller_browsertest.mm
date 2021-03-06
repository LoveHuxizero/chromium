// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/ui/cocoa/profile_signin_confirmation_view_controller.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/sync/profile_signin_confirmation_helper.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "grit/chromium_strings.h"
#include "grit/generated_resources.h"
#import "testing/gtest_mac.h"
#include "ui/base/l10n/l10n_util.h"

class ProfileSigninConfirmationViewControllerTest
  : public InProcessBrowserTest,
    public ui::ProfileSigninConfirmationDelegate {

 public:
  ProfileSigninConfirmationViewControllerTest()
    : window_(nil),
      continued_(false),
      cancelled_(false),
      created_(false),
      closed_(false) {
  }

 protected:
  virtual void SetUpOnMainThread() OVERRIDE {
  }

  void SetupDialog(bool offerProfileCreation = true) {
    window_.reset(
        [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 400, 600)
                                    styleMask:NSBorderlessWindowMask
                                      backing:NSBackingStoreBuffered
                                        defer:NO]);
    base::Closure close = base::Bind(
        &ProfileSigninConfirmationViewControllerTest::OnClose, this);
    controller_.reset([[ProfileSigninConfirmationViewController alloc]
                        initWithBrowser:browser()
                               username:username()
                               delegate:this
                    closeDialogCallback:close
                   offerProfileCreation:offerProfileCreation]);
    [[window_ contentView] addSubview:[controller_ view]];
    [window_ makeKeyAndOrderFront:NSApp];
    ASSERT_TRUE([window_ isVisible]);
  }

  // Test helpers.
  std::string username() {
    return "foo@bar.com";
  }
  base::string16 learn_more() {
    return l10n_util::GetStringUTF16(
        IDS_ENTERPRISE_SIGNIN_PROFILE_LINK_LEARN_MORE);
  }

  // ui::ProfileSigninConfirmationDelegate:
  virtual void OnContinueSignin() OVERRIDE { continued_ = true; }
  virtual void OnCancelSignin() OVERRIDE { cancelled_ = true; }
  virtual void OnSigninWithNewProfile() OVERRIDE { created_ = true; }
  void OnClose() { closed_ = true; }

  // The window containing the dialog.
  base::scoped_nsobject<NSWindow> window_;

  // Dialog under test.
  base::scoped_nsobject<ProfileSigninConfirmationViewController> controller_;

  // Visible for testing UI interactions.
  bool continued_;
  bool cancelled_;
  bool created_;
  bool closed_;

 private:
  DISALLOW_COPY_AND_ASSIGN(ProfileSigninConfirmationViewControllerTest);
};

IN_PROC_BROWSER_TEST_F(ProfileSigninConfirmationViewControllerTest,
                       ContinueClicked) {
  SetupDialog();

  // Click should invoke and clear delegate and close the dialog.
  [controller_ ok:nil];
  EXPECT_TRUE(continued_);
  EXPECT_TRUE(closed_);
  EXPECT_FALSE([controller_ delegate]);

  // Another click should have no effect.
  continued_ = false;
  closed_ = false;
  [controller_ ok:nil];
  EXPECT_FALSE(continued_);
  EXPECT_FALSE(closed_);
}

IN_PROC_BROWSER_TEST_F(ProfileSigninConfirmationViewControllerTest,
                       CancelClicked) {
  SetupDialog();

  // Click should invoke and clear delegate and close the dialog.
  [controller_ cancel:nil];
  EXPECT_TRUE(cancelled_);
  EXPECT_TRUE(closed_);
  EXPECT_FALSE([controller_ delegate]);

  // Another click should have no effect.
  cancelled_ = false;
  closed_ = false;
  [controller_ cancel:nil];
  EXPECT_FALSE(cancelled_);
  EXPECT_FALSE(closed_);
}

IN_PROC_BROWSER_TEST_F(ProfileSigninConfirmationViewControllerTest,
                       CreateClicked) {
  SetupDialog();

  // Click should invoke and clear delegate and close the dialog.
  [controller_ createProfile:nil];
  EXPECT_TRUE(created_);
  EXPECT_TRUE(closed_);
  EXPECT_FALSE([controller_ delegate]);

  // Another click should have no effect.
  created_ = false;
  closed_ = false;
  [controller_ createProfile:nil];
  EXPECT_FALSE(created_);
  EXPECT_FALSE(closed_);
}

IN_PROC_BROWSER_TEST_F(ProfileSigninConfirmationViewControllerTest,
                       CloseClicked) {
  SetupDialog();

  // Click should invoke and clear delegate and close the dialog.
  [controller_ cancel:nil];
  EXPECT_TRUE(cancelled_);
  EXPECT_TRUE(closed_);
  EXPECT_FALSE([controller_ delegate]);

  // Another click should close the dialog but not invoke the delegate.
  cancelled_ = false;
  closed_ = false;
  [controller_ close:nil];
  EXPECT_FALSE(cancelled_);
  EXPECT_TRUE(closed_);
}

IN_PROC_BROWSER_TEST_F(ProfileSigninConfirmationViewControllerTest,
                       DoNotOfferNewProfile) {
  SetupDialog(/*offerProfileCreation = */ false);
  // Create Profile button shouldn't exist.
  EXPECT_NSEQ(nil, [controller_ createProfileButton]);
  // Explanation shouldn't mention creating a new profile.
  NSString* explanationWithoutCreateProfile = base::SysUTF16ToNSString(
      l10n_util::GetStringFUTF16(
          IDS_ENTERPRISE_SIGNIN_EXPLANATION_WITHOUT_PROFILE_CREATION_NEW_STYLE,
          base::UTF8ToUTF16(username()), learn_more()));
  EXPECT_NSEQ(explanationWithoutCreateProfile,
              [[[controller_ explanationField] textStorage] string]);
}

IN_PROC_BROWSER_TEST_F(ProfileSigninConfirmationViewControllerTest,
                       OfferNewProfile) {
  SetupDialog(/*offerProfileCreation = */ true);
  // Create Profile button should exist and be visible.
  EXPECT_NSNE(nil, [controller_ createProfileButton]);
  EXPECT_TRUE([[[controller_ view] subviews]
                containsObject:[controller_ createProfileButton]]);
  NSString* explanationWithCreateProfile = base::SysUTF16ToNSString(
      l10n_util::GetStringFUTF16(
          IDS_ENTERPRISE_SIGNIN_EXPLANATION_WITH_PROFILE_CREATION_NEW_STYLE,
          base::UTF8ToUTF16(username()), learn_more()));
  EXPECT_NSEQ(explanationWithCreateProfile,
              [[[controller_ explanationField] textStorage] string]);
}
