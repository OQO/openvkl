// Copyright 2019-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "DefaultIterator.h"
#include "../common/export_util.h"
#include "../common/math.h"
#include "../value_selector/ValueSelector.h"
#include "../volume/Volume.h"
#include "DefaultIterator_ispc.h"

namespace openvkl {
  namespace ispc_driver {

    ////////////////////////////////////////////////////////////////////////////

    template <int W>
    void DefaultIntervalIterator<W>::initializeIntervalV(
        const vintn<W> &valid,
        const vvec3fn<W> &origin,
        const vvec3fn<W> &direction,
        const vrange1fn<W> &tRange,
        const ValueSelector<W> *valueSelector)
    {
      box3f boundingBox  = volume->getBoundingBox();
      range1f valueRange = volume->getValueRange();

      CALL_ISPC(DefaultIterator_Initialize,
                static_cast<const int *>(valid),
                ispcStorage,
                volume->getISPCEquivalent(),
                (void *)&origin,
                (void *)&direction,
                (void *)&tRange,
                valueSelector ? valueSelector->getISPCEquivalent() : nullptr,
                (const ispc::box3f &)boundingBox,
                (const ispc::box1f &)valueRange);
    }

    template <int W>
    void DefaultIntervalIterator<W>::iterateIntervalV(
        const vintn<W> &valid, vVKLIntervalN<W> &interval, vintn<W> &result)
    {
      CALL_ISPC(DefaultIterator_iterateInterval,
                static_cast<const int *>(valid),
                ispcStorage,
                &interval,
                static_cast<int *>(result));
    }

    template class DefaultIntervalIterator<VKL_TARGET_WIDTH>;

    ////////////////////////////////////////////////////////////////////////////

    template <int W>
    void DefaultHitIterator<W>::initializeHitV(
        const vintn<W> &valid,
        const vvec3fn<W> &origin,
        const vvec3fn<W> &direction,
        const vrange1fn<W> &tRange,
        const ValueSelector<W> *valueSelector)
    {
      box3f boundingBox  = volume->getBoundingBox();
      range1f valueRange = volume->getValueRange();

      CALL_ISPC(DefaultIterator_Initialize,
                static_cast<const int *>(valid),
                ispcStorage,
                volume->getISPCEquivalent(),
                (void *)&origin,
                (void *)&direction,
                (void *)&tRange,
                valueSelector ? valueSelector->getISPCEquivalent() : nullptr,
                (const ispc::box3f &)boundingBox,
                (const ispc::box1f &)valueRange);
    }

    template <int W>
    void DefaultHitIterator<W>::iterateHitV(const vintn<W> &valid,
                                            vVKLHitN<W> &hit,
                                            vintn<W> &result)
    {
      CALL_ISPC(DefaultIterator_iterateHit,
                static_cast<const int *>(valid),
                ispcStorage,
                &hit,
                static_cast<int *>(result));
    }

    template class DefaultHitIterator<VKL_TARGET_WIDTH>;

  }  // namespace ispc_driver
}  // namespace openvkl
