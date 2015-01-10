// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_LAYERS_SURFACE_LAYER_IMPL_H_
#define CC_LAYERS_SURFACE_LAYER_IMPL_H_

#include "base/memory/scoped_ptr.h"
#include "cc/base/cc_export.h"
#include "cc/layers/layer_impl.h"
#include "cc/surfaces/surface_id.h"

namespace cc {

class CC_EXPORT SurfaceLayerImpl : public LayerImpl {
 public:
  static scoped_ptr<SurfaceLayerImpl> Create(LayerTreeImpl* tree_impl, int id) {
    return make_scoped_ptr(new SurfaceLayerImpl(tree_impl, id));
  }
  ~SurfaceLayerImpl() override;

  void SetSurfaceId(SurfaceId surface_id);

  // LayerImpl overrides.
  scoped_ptr<LayerImpl> CreateLayerImpl(LayerTreeImpl* tree_impl) override;
  void PushPropertiesTo(LayerImpl* layer) override;
  void AppendQuads(RenderPass* render_pass,
                   const Occlusion& occlusion_in_content_space,
                   AppendQuadsData* append_quads_data) override;

 protected:
  SurfaceLayerImpl(LayerTreeImpl* tree_impl, int id);

 private:
  void GetDebugBorderProperties(SkColor* color, float* width) const override;
  void AsValueInto(base::debug::TracedValue* dict) const override;
  const char* LayerTypeAsString() const override;

  SurfaceId surface_id_;

  DISALLOW_COPY_AND_ASSIGN(SurfaceLayerImpl);
};

}  // namespace cc

#endif  // CC_LAYERS_SURFACE_LAYER_IMPL_H_
