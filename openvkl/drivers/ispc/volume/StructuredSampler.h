// Copyright 2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../common/export_util.h"
#include "../sampler/Sampler.h"
#include "SharedStructuredVolume_ispc.h"
#include "StructuredVolume.h"
#include "Volume_ispc.h"

namespace openvkl {
  namespace ispc_driver {

    template <int W>
    struct StructuredSampler : public Sampler<W>
    {
      StructuredSampler(const StructuredVolume<W> *volume);

      ~StructuredSampler() override = default;

      void commit() override {}

      void computeSample(const vvec3fn<1> &objectCoordinates,
                         vfloatn<1> &samples) const override final;

      void computeSampleV(const vintn<W> &valid,
                          const vvec3fn<W> &objectCoordinates,
                          vfloatn<W> &samples) const override final;

      void computeSampleN(unsigned int N,
                          const vvec3fn<1> *objectCoordinates,
                          float *samples) const override final;

      void computeGradientV(const vintn<W> &valid,
                            const vvec3fn<W> &objectCoordinates,
                            vvec3fn<W> &gradients) const override final;

      void computeGradientN(unsigned int N,
                            const vvec3fn<1> *objectCoordinates,
                            vvec3fn<1> *gradients) const override final;

     protected:
      const StructuredVolume<W> *volume{nullptr};
    };

    // Inlined definitions ////////////////////////////////////////////////////

    template <int W>
    inline StructuredSampler<W>::StructuredSampler(
        const StructuredVolume<W> *volume)
        : volume(volume)
    {
      assert(volume);
    }

    template <int W>
    inline void StructuredSampler<W>::computeSample(
        const vvec3fn<1> &objectCoordinates, vfloatn<1> &samples) const
    {
      CALL_ISPC(SharedStructuredVolume_sample_uniform_export,
                volume->getISPCEquivalent(),
                &objectCoordinates,
                &samples);
    }

    template <int W>
    inline void StructuredSampler<W>::computeSampleV(
        const vintn<W> &valid,
        const vvec3fn<W> &objectCoordinates,
        vfloatn<W> &samples) const
    {
      CALL_ISPC(SharedStructuredVolume_sample_export,
                static_cast<const int *>(valid),
                volume->getISPCEquivalent(),
                &objectCoordinates,
                &samples);
    }

    template <int W>
    inline void StructuredSampler<W>::computeSampleN(
        unsigned int N,
        const vvec3fn<1> *objectCoordinates,
        float *samples) const
    {
      CALL_ISPC(Volume_sample_N_export,
                volume->getISPCEquivalent(),
                N,
                (ispc::vec3f *)objectCoordinates,
                samples);
    }

    template <int W>
    inline void StructuredSampler<W>::computeGradientV(
        const vintn<W> &valid,
        const vvec3fn<W> &objectCoordinates,
        vvec3fn<W> &gradients) const
    {
      CALL_ISPC(SharedStructuredVolume_gradient_export,
                static_cast<const int *>(valid),
                volume->getISPCEquivalent(),
                &objectCoordinates,
                &gradients);
    }

    template <int W>
    inline void StructuredSampler<W>::computeGradientN(
        unsigned int N,
        const vvec3fn<1> *objectCoordinates,
        vvec3fn<1> *gradients) const
    {
      CALL_ISPC(Volume_gradient_N_export,
                volume->getISPCEquivalent(),
                N,
                (ispc::vec3f *)objectCoordinates,
                (ispc::vec3f *)gradients);
    }

  }  // namespace ispc_driver
}  // namespace openvkl
