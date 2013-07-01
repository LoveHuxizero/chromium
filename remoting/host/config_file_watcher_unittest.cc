// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/config_file_watcher.h"

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/message_loop.h"
#include "base/run_loop.h"
#include "remoting/base/auto_thread.h"
#include "remoting/base/auto_thread_task_runner.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gmock_mutant.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;
using testing::AnyNumber;
using testing::Return;

namespace remoting {

namespace {

class ConfigFileWatcherDelegate : public ConfigFileWatcher::Delegate {
 public:
  ConfigFileWatcherDelegate() {}
  virtual ~ConfigFileWatcherDelegate() {}

  MOCK_METHOD1(OnConfigUpdated, void(const std::string&));
  MOCK_METHOD0(OnConfigWatcherError, void());

 private:
  DISALLOW_COPY_AND_ASSIGN(ConfigFileWatcherDelegate);
};

}  // namespace

class ConfigFileWatcherTest : public testing::Test {
 public:
  ConfigFileWatcherTest();
  virtual ~ConfigFileWatcherTest();

  // testing::Test overrides
  virtual void SetUp() OVERRIDE;
  virtual void TearDown() OVERRIDE;

  // Stops the config file watcher.
  void StopWatcher();

 protected:
  base::MessageLoop message_loop_;
  base::RunLoop run_loop_;

  ConfigFileWatcherDelegate delegate_;

  // Path to the configuration file used.
  base::FilePath config_file_;

  // The configuration file watcher that is being tested.
  scoped_ptr<ConfigFileWatcher> watcher_;
};

ConfigFileWatcherTest::ConfigFileWatcherTest()
    : message_loop_(base::MessageLoop::TYPE_UI) {
}

ConfigFileWatcherTest::~ConfigFileWatcherTest() {
}

void ConfigFileWatcherTest::StopWatcher() {
  watcher_.reset();
}

void ConfigFileWatcherTest::SetUp() {
  // Arrange to run |message_loop_| until no components depend on it.
  scoped_refptr<AutoThreadTaskRunner> task_runner = new AutoThreadTaskRunner(
      message_loop_.message_loop_proxy(), run_loop_.QuitClosure());

  scoped_refptr<AutoThreadTaskRunner> io_task_runner =
      AutoThread::CreateWithType(
          "IPC thread", task_runner, base::MessageLoop::TYPE_IO);

  // Create an instance of the config watcher.
  watcher_.reset(
      new ConfigFileWatcher(task_runner, io_task_runner, &delegate_));
}

void ConfigFileWatcherTest::TearDown() {
  // Delete the test file.
  if (!config_file_.empty())
    base::Delete(config_file_, false);
}

// Verifies that the initial notification is delivered.
TEST_F(ConfigFileWatcherTest, Basic) {
  EXPECT_TRUE(file_util::CreateTemporaryFile(&config_file_));

  std::string data("test");
  EXPECT_NE(file_util::WriteFile(config_file_, data.c_str(),
                                 static_cast<int>(data.size())), -1);

  EXPECT_CALL(delegate_, OnConfigUpdated(_))
      .Times(1)
      .WillOnce(InvokeWithoutArgs(this, &ConfigFileWatcherTest::StopWatcher));
  EXPECT_CALL(delegate_, OnConfigWatcherError())
      .Times(0);

  watcher_->Watch(config_file_);
  run_loop_.Run();
}

MATCHER_P(EqualsString, s, "") {
  return arg == s;
}

// Verifies that an update notification is sent when the file is changed.
TEST_F(ConfigFileWatcherTest, Update) {
  EXPECT_TRUE(file_util::CreateTemporaryFile(&config_file_));

  EXPECT_CALL(delegate_, OnConfigUpdated(EqualsString("test")))
      .Times(1)
      .WillOnce(InvokeWithoutArgs(this, &ConfigFileWatcherTest::StopWatcher));
  EXPECT_CALL(delegate_, OnConfigWatcherError())
      .Times(0);

  watcher_->Watch(config_file_);

  // Modify the watched file.
  std::string data("test");
  EXPECT_NE(file_util::WriteFile(config_file_, data.c_str(),
                                 static_cast<int>(data.size())), -1);

  run_loop_.Run();
}

}  // namespace remoting
