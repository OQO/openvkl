// Copyright 2019-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../common/math.h"
#include "Iterator.h"
#include "DefaultIterator.h"
#include "UnstructuredIterator_ispc.h"

namespace openvkl {
  namespace ispc_driver {

    template <int W>
    struct UnstructuredIntervalIterator : public IntervalIterator<W>
    {
      using IntervalIterator<W>::IntervalIterator;

      void initializeIntervalV(
          const vintn<W> &valid,
          const vvec3fn<W> &origin,
          const vvec3fn<W> &direction,
          const vrange1fn<W> &tRange,
          const ValueSelector<W> *valueSelector) override final;

      void iterateIntervalV(const vintn<W> &valid,
                            vVKLIntervalN<W> &interval,
                            vintn<W> &result) override final;

      void *getIspcStorage() override final
      {
        return reinterpret_cast<void*>(ispcStorage);
      }

     protected:
      using Iterator<W>::volume;
      using IspcIterator = __varying_ispc_type(UnstructuredIterator);
      alignas(alignof(IspcIterator)) char ispcStorage[sizeof(IspcIterator)];
    };

    template <int W>
    using UnstructuredIntervalIteratorFactory =
        ConcreteIteratorFactory<W,
                                IntervalIterator,
                                UnstructuredIntervalIterator>;

    template <int W>
    using UnstructuredHitIterator = DefaultHitIterator<W, UnstructuredIntervalIterator<W>>;

    template <int W>
    using UnstructuredHitIteratorFactory =
        ConcreteIteratorFactory<W, HitIterator, UnstructuredHitIterator>;

  }  // namespace ispc_driver
}  // namespace openvkl
