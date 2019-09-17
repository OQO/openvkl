// ======================================================================== //
// Copyright 2009-2019 Intel Corporation                                    //
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

#pragma once

#include "../StructuredVolume.h"
#include "AMRAccel.h"
#include "ospcommon/memory/RefCount.h"

using namespace ospcommon::memory;

typedef enum
{
  VKL_AMR_CURRENT,
  VKL_AMR_FINEST,
  VKL_AMR_OCTANT
} VKLAMRMethod;

namespace openvkl {
  namespace ispc_driver {

    template <int W>
    struct AMRVolume : public StructuredVolume<W>
    {
      AMRVolume();
      ~AMRVolume() override = default;

      std::string toString() const override;

      void commit() override;

      void computeSampleV(const int *valid,
                          const vvec3fn<W> &objectCoordinates,
                          vfloatn<W> &samples) const override;
      vec3f computeGradient(const vec3f &objectCoordinates) const override;
      box3f getBoundingBox() const override;

      std::unique_ptr<amr::AMRData> data;
      std::unique_ptr<amr::AMRAccel> accel;

      Ref<Data> blockDataData;
      Ref<Data> blockBoundsData;
      Ref<Data> refinementLevelsData;
      Ref<Data> cellWidthsData;
      VKLDataType voxelType;
      vec2f voxelRange;
      box3f bounds;

      VKLAMRMethod amrMethod;

      void *ispcEquivalent{nullptr};
    };

  }  // namespace ispc_driver
}  // namespace openvkl