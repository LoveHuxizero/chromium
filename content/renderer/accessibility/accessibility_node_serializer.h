// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_ACCESSIBILITY_ACCESSIBILITY_NODE_SERIALIZER_H_
#define CONTENT_RENDERER_ACCESSIBILITY_ACCESSIBILITY_NODE_SERIALIZER_H_

#include "third_party/WebKit/public/web/WebAXObject.h"
#include "ui/accessibility/ax_node_data.h"

namespace content {

void SerializeAccessibilityNode(
    const blink::WebAXObject& src,
    ui::AXNodeData* dst);

bool ShouldIncludeChildNode(
    const blink::WebAXObject& parent,
    const blink::WebAXObject& child);

}  // namespace content

#endif  // CONTENT_RENDERER_ACCESSIBILITY_ACCESSIBILITY_NODE_SERIALIZER_H_
