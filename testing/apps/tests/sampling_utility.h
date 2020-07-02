// Copyright 2019-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../../external/catch.hpp"
#include "aos_soa_conversion.h"
#include "openvkl_testing.h"

inline void test_scalar_and_vector_sampling(VKLSampler sampler,
                                            const vec3f &objectCoordinates,
                                            const float sampleTruth,
                                            const float sampleTolerance)
{
  float scalarSampledValue =
      vklComputeSample(sampler, (const vkl_vec3f *)&objectCoordinates);

  REQUIRE(scalarSampledValue == Approx(sampleTruth).margin(sampleTolerance));

  // since vklComputeSample() can have a specialized implementation separate
  // from vector sampling, check the vector APIs as well. we only need to
  // check for consistency with the scalar API result, as that has already
  // been validated.

  // first lane active only
  std::vector<int> valid(16, 0);
  valid[0] = 1;

  std::vector<vec3f> objectCoordinatesVector;
  objectCoordinatesVector.push_back(objectCoordinates);

  AlignedVector<float> objectCoordinatesSOA;

  objectCoordinatesSOA = AOStoSOA_vec3f(objectCoordinatesVector, 4);
  float samples_4[4]   = {0.f};
  vklComputeSample4(valid.data(),
                    sampler,
                    (const vkl_vvec3f4 *)objectCoordinatesSOA.data(),
                    samples_4);

  objectCoordinatesSOA = AOStoSOA_vec3f(objectCoordinatesVector, 8);
  float samples_8[8]   = {0.f};
  vklComputeSample8(valid.data(),
                    sampler,
                    (const vkl_vvec3f8 *)objectCoordinatesSOA.data(),
                    samples_8);

  objectCoordinatesSOA = AOStoSOA_vec3f(objectCoordinatesVector, 16);
  float samples_16[16] = {0.f};
  vklComputeSample16(valid.data(),
                     sampler,
                     (const vkl_vvec3f16 *)objectCoordinatesSOA.data(),
                     samples_16);

  REQUIRE(scalarSampledValue == samples_4[0]);
  REQUIRE(scalarSampledValue == samples_8[0]);
  REQUIRE(scalarSampledValue == samples_16[0]);
}