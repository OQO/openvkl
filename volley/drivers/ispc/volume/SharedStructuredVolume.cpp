// ======================================================================== //
// Copyright 2018 Intel Corporation                                         //
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

#include "SharedStructuredVolume.h"
#include <ospray/ospcommon/tasking/parallel_for.h>
#include "GridAccelerator_ispc.h"

namespace volley {
  namespace ispc_driver {

    SharedStructuredVolume::~SharedStructuredVolume()
    {
      if (ispcEquivalent) {
        ispc::SharedStructuredVolume_Destructor(ispcEquivalent);
      }
    }

    void SharedStructuredVolume::commit()
    {
      StructuredVolume::commit();

      if (!ispcEquivalent) {
        ispcEquivalent = ispc::SharedStructuredVolume_Constructor();
      }

      ispc::SharedStructuredVolume_set(ispcEquivalent,
                                       volumeData.data(),
                                       (const ispc::vec3i &)dimensions,
                                       (const ispc::vec3f &)gridOrigin,
                                       (const ispc::vec3f &)gridSpacing);

      buildAccelerator();
    }

    void SharedStructuredVolume::buildAccelerator()
    {
      void *accelerator =
          ispc::SharedStructuredVolume_createAccelerator(ispcEquivalent);

      vec3i brickCount;
      brickCount.x = ispc::GridAccelerator_getBrickCount_x(accelerator);
      brickCount.y = ispc::GridAccelerator_getBrickCount_y(accelerator);
      brickCount.z = ispc::GridAccelerator_getBrickCount_z(accelerator);

      const int NTASKS = brickCount.x * brickCount.y * brickCount.z;
      tasking::parallel_for(NTASKS, [&](int taskIndex) {
        ispc::GridAccelerator_buildAccelerator(ispcEquivalent, taskIndex);
      });
    }

    VLY_REGISTER_VOLUME(SharedStructuredVolume, shared_structured_volume)

  }  // namespace ispc_driver
}  // namespace volley