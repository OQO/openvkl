// Copyright 2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vector>
// openvkl
#include "TestingVolume.h"
// rkcommon
#include <random>
#include "rkcommon/math/range.h"
#include "rkcommon/math/vec.h"

namespace openvkl {
  namespace testing {

    struct ProceduralParticleVolume : public TestingVolume
    {
      ProceduralParticleVolume(size_t numParticles,
                               bool provideWeights           = true,
                               float radiusSupportFactor     = 3.f,
                               float clampMaxCumulativeValue = 0.f,
                               bool estimateValueRanges      = true);

      range1f getComputedValueRange() const override;

      float computeReferenceSample(const vec3f &p);
      vec3f computeReferenceGradient(const vec3f &p);

      const std::vector<vec4f> &getParticles();

     protected:
      void generateVKLVolume() override;

      size_t numParticles;
      bool provideWeights;
      float radiusSupportFactor;
      float clampMaxCumulativeValue;
      bool estimateValueRanges;

      // particles will be seeded within these bounds
      box3f bounds = box3f(-1.f, 1.f);

      std::vector<vec4f> particles;  // position (x, y, z) and radius (w)
      std::vector<float> weights;
      range1f computedValueRange = range1f(rkcommon::math::empty);
    };

    // Inlined definitions ////////////////////////////////////////////////////

    inline ProceduralParticleVolume::ProceduralParticleVolume(
        size_t numParticles,
        bool provideWeights,
        float radiusSupportFactor,
        float clampMaxCumulativeValue,
        bool estimateValueRanges)
        : numParticles(numParticles),
          provideWeights(provideWeights),
          radiusSupportFactor(radiusSupportFactor),
          clampMaxCumulativeValue(clampMaxCumulativeValue),
          estimateValueRanges(estimateValueRanges)
    {
    }

    inline range1f ProceduralParticleVolume::getComputedValueRange() const
    {
      if (computedValueRange.empty()) {
        throw std::runtime_error(
            "computedValueRange only available after VKL volume is generated");
      }

      return computedValueRange;
    }

    inline float ProceduralParticleVolume::computeReferenceSample(
        const vec3f &p)
    {
      float referenceSample = 0.f;

      for (size_t j = 0; j < particles.size(); j++) {
        const vec4f &pj = particles[j];
        const float wj  = weights[j];

        // This should match whichever RBF is used in ParticleVolume.ispc
        // Currently, we use Gaussian.
        const vec3f center(pj.x, pj.y, pj.z);
        const vec3f distance = p - center;

        if (length(distance) > pj.w * radiusSupportFactor)
          continue;

        const float kernelValue =
            wj * expf(-0.5f * dot(distance, distance) / (pj.w * pj.w));

        referenceSample += kernelValue;
      }

      if (clampMaxCumulativeValue > 0.f) {
        return std::min(referenceSample, clampMaxCumulativeValue);
      } else {
        return referenceSample;
      }
    }

    inline vec3f ProceduralParticleVolume::computeReferenceGradient(
        const vec3f &objectCoordinates)
    {
      const vec3f gradientStep(1e-5f);

      vec3f gradient;

      const float sample = computeReferenceSample(objectCoordinates);

      gradient.x = computeReferenceSample(objectCoordinates +
                                          vec3f(gradientStep.x, 0.f, 0.f)) -
                   sample;
      gradient.y = computeReferenceSample(objectCoordinates +
                                          vec3f(0.f, gradientStep.y, 0.f)) -
                   sample;
      gradient.z = computeReferenceSample(objectCoordinates +
                                          vec3f(0.f, 0.f, gradientStep.z)) -
                   sample;

      return gradient / gradientStep;
    }

    inline const std::vector<vec4f> &ProceduralParticleVolume::getParticles()
    {
      return particles;
    }

    inline void ProceduralParticleVolume::generateVKLVolume()
    {
      int32_t randomSeed = 0;

      // create random number distributions for point center and weight
      std::mt19937 gen(randomSeed);

      range1f weightRange(0.5f, 1.5f);

      const float radiusScale = 1.f / powf(numParticles, 1.f / 3.f);

      std::uniform_real_distribution<float> centerDistribution_x(
          bounds.lower.x, bounds.upper.x);
      std::uniform_real_distribution<float> centerDistribution_y(
          bounds.lower.y, bounds.upper.y);
      std::uniform_real_distribution<float> centerDistribution_z(
          bounds.lower.z, bounds.upper.z);
      std::uniform_real_distribution<float> radiusDistribution(
          .25f * radiusScale, 1.f * radiusScale);

      std::uniform_real_distribution<float> weightDistribution(
          weightRange.lower, weightRange.upper);

      // populate the particles
      particles.resize(numParticles);
      weights.resize(numParticles);

      // random particles
      for (int i = 0; i < numParticles; i++) {
        auto &p    = particles[i];
        p.x        = centerDistribution_x(gen);
        p.y        = centerDistribution_y(gen);
        p.z        = centerDistribution_z(gen);
        p.w        = radiusDistribution(gen);
        weights[i] = provideWeights ? weightDistribution(gen) : 1.f;
      }

      volume = vklNewVolume("particle");

      VKLData positionsData = vklNewData(numParticles,
                                         VKL_VEC3F,
                                         particles.data(),
                                         VKL_DATA_SHARED_BUFFER,
                                         sizeof(vec4f));
      vklSetData(volume, "particle.position", positionsData);
      vklRelease(positionsData);

      VKLData radiiData = vklNewData(numParticles,
                                     VKL_FLOAT,
                                     &(particles.data()[0].w),
                                     VKL_DATA_SHARED_BUFFER,
                                     sizeof(vec4f));
      vklSetData(volume, "particle.radius", radiiData);
      vklRelease(radiiData);

      if (provideWeights) {
        VKLData weightsData =
            vklNewData(numParticles, VKL_FLOAT, weights.data());
        vklSetData(volume, "particle.weight", weightsData);
        vklRelease(weightsData);
      }

      vklSetFloat(volume, "radiusSupportFactor", radiusSupportFactor);
      vklSetFloat(volume, "clampMaxCumulativeValue", clampMaxCumulativeValue);
      vklSetBool(volume, "estimateValueRanges", estimateValueRanges);

      vklCommit(volume);

      // compute value range

      // initial estimate based only on weights of individual particles (not
      // considering overlap)
      computedValueRange =
          computeValueRange(VKL_FLOAT, weights.data(), weights.size());

      if (clampMaxCumulativeValue > 0.f) {
        const range1f legalRange(0.f, clampMaxCumulativeValue);

        computedValueRange =
            range1f(legalRange.clamp(computedValueRange.lower),
                    legalRange.clamp(computedValueRange.upper));
      }

      // Sample over regular grid to improve estimate. Note that we use
      // vklComputeSample() for this instead of computeReferenceSample(),
      // for performance. The correctness of sampling / equivalence of these
      // two methods is validated in the sampling functional tests.
      const int samplesPerDimension = 100;

      VKLSampler sampler = vklNewSampler(volume);
      vklCommit(sampler);

      for (int z = 0; z < samplesPerDimension; z++) {
        for (int y = 0; y < samplesPerDimension; y++) {
          for (int x = 0; x < samplesPerDimension; x++) {
            vec3f objectCoordinates =
                bounds.lower +
                vec3f(x, y, z) / float(samplesPerDimension - 1) * bounds.size();

            computedValueRange.extend(vklComputeSample(
                sampler, (const vkl_vec3f *)&objectCoordinates));
          }
        }
      };

      vklRelease(sampler);
    }

  }  // namespace testing
}  // namespace openvkl
