// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_INPUT_LAYER_SELECTION_BOUND_H_
#define CC_INPUT_LAYER_SELECTION_BOUND_H_

#include "cc/base/cc_export.h"
#include "cc/input/selection_bound_type.h"
#include "ui/gfx/geometry/rect_f.h"

namespace cc {

// Marker for a selection end-point attached to a specific layer.
struct CC_EXPORT LayerSelectionBound {
  LayerSelectionBound();
  ~LayerSelectionBound();

  SelectionBoundType type;
  gfx::RectF layer_rect;
  int layer_id;
};

bool operator==(const LayerSelectionBound& lhs, const LayerSelectionBound& rhs);
bool operator!=(const LayerSelectionBound& lhs, const LayerSelectionBound& rhs);

}  // namespace cc

#endif  // CC_INPUT_LAYER_SELECTION_BOUND_H_
