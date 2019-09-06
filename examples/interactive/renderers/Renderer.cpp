// ======================================================================== //
// Copyright 2019 Intel Corporation                                         //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "Renderer.h"
// std
#include <algorithm>
// ospcommon
#include "ospcommon/tasking/parallel_for.h"
// ispc
#include "Renderer_ispc.h"

namespace openvkl {
  namespace examples {

    Renderer::~Renderer()
    {
      ispc::Renderer_freeRenderer(ispcEquivalent);
    }

    void Renderer::commit()
    {
      spp = getParam<int>("spp", 1);

      setTransferFunction(tfValueRange, tfColorsAndOpacities);
    }

    void Renderer::setCamera(const vec3f &pos,
                             const vec3f &dir,
                             const vec3f &up,
                             float aspect,
                             float fovy)
    {
      camPos = pos;

      dir_du = normalize(cross(dir, up));
      dir_dv = cross(dir_du, dir);

      float imgPlane_size_y = 2.f * tanf(fovy / 2.f * M_PI / 180.);
      float imgPlane_size_x = imgPlane_size_y * aspect;

      dir_du *= imgPlane_size_x;
      dir_dv *= imgPlane_size_y;

      dir_00 = dir - .5f * dir_du - .5f * dir_dv;

      ispc::Renderer_setCamera(ispcEquivalent,
                               (ispc::vec3f &)camPos,
                               (ispc::vec3f &)dir_00,
                               (ispc::vec3f &)dir_du,
                               (ispc::vec3f &)dir_dv);
    }

    void Renderer::setFrameSize(const vec2i &dims)
    {
      pixelIndices = index_sequence_2D(dims);

      const auto numPixels = pixelIndices.total_indices();
      framebuffer.resize(numPixels);
      accum_r.resize(numPixels);
      accum_g.resize(numPixels);
      accum_b.resize(numPixels);

      ispc::Renderer_setFrameBuffer(ispcEquivalent,
                                    (ispc::vec3f *)framebuffer.data(),
                                    accum_r.data(),
                                    accum_g.data(),
                                    accum_b.data());
    }

    vec2i Renderer::frameSize() const
    {
      return pixelIndices.dimensions();
    }

    void Renderer::resetAccumulation()
    {
      std::fill(accum_r.begin(), accum_r.end(), 0.f);
      std::fill(accum_g.begin(), accum_g.end(), 0.f);
      std::fill(accum_b.begin(), accum_b.end(), 0.f);
      frameID = 0;

      ispc::Renderer_setFrameID(ispcEquivalent, frameID);
    }

    const FrameBuffer &Renderer::frameBuffer() const
    {
      return framebuffer;
    }

    void Renderer::setTransferFunction(
        const range1f &valueRange, const std::vector<vec4f> &colorsAndOpacities)
    {
      tfValueRange         = valueRange;
      tfColorsAndOpacities = colorsAndOpacities;

      ispc::Renderer_setTransferFunction(
          ispcEquivalent,
          (ispc::vec2f &)tfValueRange,
          tfColorsAndOpacities.size(),
          (ispc::vec4f *)tfColorsAndOpacities.data());
    }

    void Renderer::renderFrame(VKLVolume volume, VKLSamplesMask mask)
    {
      auto fbDims = pixelIndices.dimensions();

      const auto vkl_bounds    = vklGetBoundingBox(volume);
      const box3f volumeBounds = (const box3f &)vkl_bounds;

      for (int i = 0; i < spp; ++i) {
        float accumScale = 1.f / (frameID + 1);

        tasking::parallel_for(pixelIndices.total_indices(), [&](size_t i) {
          auto pixel = pixelIndices.reshape(i);

          vec2f screen(pixel.x * rcp(float(fbDims.x)),
                       pixel.y * rcp(float(fbDims.y)));

          Ray ray = computeRay(screen);

          vec3f color = renderPixel(volume,
                                    volumeBounds,
                                    mask,
                                    ray,
                                    vec4i(pixel.x, pixel.y, frameID, fbDims.x));

          float &ar = accum_r[i];
          float &ag = accum_g[i];
          float &ab = accum_b[i];

          ar += color.x;
          ag += color.y;
          ab += color.z;

          framebuffer[i] = vec3f(ar, ag, ab) * accumScale;

          // linear to sRGB color space conversion
          framebuffer[i] = vec3f(pow(framebuffer[i].x, 1.f / 2.2f),
                                 pow(framebuffer[i].y, 1.f / 2.2f),
                                 pow(framebuffer[i].z, 1.f / 2.2f));
        });

        frameID++;
      }
    }

    void Renderer::renderFrame_ispc(VKLVolume volume, VKLSamplesMask mask)
    {
      auto fbDims = pixelIndices.dimensions();

      const auto vkl_bounds    = vklGetBoundingBox(volume);
      const box3f volumeBounds = (const box3f &)vkl_bounds;

      const size_t numJobs =
          pixelIndices.total_indices() / ispc::Renderer_pixelsPerJob();

      for (int i = 0; i < spp; ++i) {
        float accumScale = 1.f / (frameID + 1);

        tasking::parallel_for(numJobs, [&](size_t i) {
          ispc::Renderer_renderPixel(ispcEquivalent,
                                     volume,
                                     (ispc::box3f &)volumeBounds,
                                     mask,
                                     (ispc::vec2i &)fbDims,
                                     frameID,
                                     accumScale,
                                     i);
        });

        frameID++;
      }
    }

  }  // namespace examples
}  // namespace openvkl