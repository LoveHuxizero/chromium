// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOOLS_GN_ITEM_H_
#define TOOLS_GN_ITEM_H_

#include <string>

#include "tools/gn/label.h"
#include "tools/gn/visibility.h"

class Config;
class ParseNode;
class Settings;
class Target;
class Toolchain;

// A named item (target, config, etc.) that participates in the dependency
// graph.
class Item {
 public:
  Item(const Settings* settings, const Label& label);
  virtual ~Item();

  const Settings* settings() const { return settings_; }

  // This is guaranteed to never change after construction so this can be
  // accessed from any thread with no locking once the item is constructed.
  const Label& label() const { return label_; }

  const ParseNode* defined_from() const { return defined_from_; }
  void set_defined_from(const ParseNode* df) { defined_from_ = df; }

  Visibility& visibility() { return visibility_; }
  const Visibility& visibility() const { return visibility_; }

  // Manual RTTI.
  virtual Config* AsConfig();
  virtual const Config* AsConfig() const;
  virtual Target* AsTarget();
  virtual const Target* AsTarget() const;
  virtual Toolchain* AsToolchain();
  virtual const Toolchain* AsToolchain() const;

  // Returns a name like "target" or "config" for the type of item this is, to
  // be used in logging and error messages.
  std::string GetItemTypeName() const;

  // Called when this item is resolved, meaning it and all of its dependents
  // have no unresolved deps.
  virtual void OnResolved() {}

 private:
  const Settings* settings_;
  Label label_;
  const ParseNode* defined_from_;

  Visibility visibility_;
};

#endif  // TOOLS_GN_ITEM_H_
