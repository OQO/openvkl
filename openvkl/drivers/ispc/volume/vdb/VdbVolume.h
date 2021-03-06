// Copyright 2019-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <openvkl/vdb.h>
#include <memory>
#include "../StructuredVolume.h"
#include "../common/Data.h"
#include "VdbGrid.h"
#include "VdbIterator.h"
#include "VdbSampleConfig.h"
#include "VdbVolume_ispc.h"
#include "rkcommon/memory/RefCount.h"

using namespace rkcommon::memory;

namespace openvkl {
  namespace ispc_driver {

    struct AlignedISPCData1D
    {
      // to be represented as VDB leaf pointers, addresses must be 16-byte
      // aligned
      alignas(16) ispc::Data1D data;
    };

    static_assert(sizeof(AlignedISPCData1D) == sizeof(ispc::Data1D),
                  "AlignedISPCData1D has incorrect size");

    template <int W>
    struct VdbVolume : public Volume<W>
    {
      VdbVolume(const VdbVolume &) = delete;
      VdbVolume &operator=(const VdbVolume &) = delete;

      VdbVolume();
      VdbVolume(VdbVolume &&other);
      VdbVolume &operator=(VdbVolume &&other);
      ~VdbVolume() override;

      /*
       * Return "openvkl::VdbVolume".
       */
      std::string toString() const override;

      /*
       * Commit the volume after setup, but before rendering.
       * Will build the main tree structure from all leaves
       * provided as parameters.
       */
      void commit() override;

      /*
       * Obtain the volume bounding box.
       */
      box3f getBoundingBox() const override
      {
        return bounds;
      }

      /*
       * Get the minimum and maximum value in this volume.
       */
      range1f getValueRange() const override
      {
        return valueRange;
      }

      const VdbGrid *getGrid() const
      {
        return grid;
      }

      VKLObserver newObserver(const char *type) override;
      Sampler<W> *newSampler() override;

      const IteratorFactory<W, IntervalIterator> &getIntervalIteratorFactory()
          const override final
      {
        return intervalIteratorFactory;
      }

      const IteratorFactory<W, HitIterator> &getHitIteratorFactory()
          const override final
      {
        return hitIteratorFactory;
      }

     private:
      void cleanup();

     private:
      box3f bounds;
      std::string name;
      range1f valueRange;
      Ref<const DataT<Data *>> leafData;
      std::vector<AlignedISPCData1D> leafDataISPC;
      VdbGrid *grid{nullptr};
      size_t bytesAllocated{0};
      VdbSampleConfig globalConfig;
      VdbIntervalIteratorFactory<W> intervalIteratorFactory;
      VdbHitIteratorFactory<W> hitIteratorFactory;
    };

  }  // namespace ispc_driver
}  // namespace openvkl
