// Copyright 2019-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "benchmark/benchmark.h"
#include "openvkl_testing.h"
#include "benchmark_suite/volume.h"

using namespace openvkl::testing;
using namespace rkcommon::utility;
using openvkl::testing::WaveletStructuredRegularVolume;

/*
 * Structured volume wrapper.
 */
struct Structured
{
  static std::string name()
  {
    return std::string();
  }

  Structured()
  {
    volume = rkcommon::make_unique<WaveletStructuredRegularVolume<float>>(
        vec3i(128), vec3f(0.f), vec3f(1.f));

    vklVolume  = volume->getVKLVolume();
    vklSampler = vklNewSampler(vklVolume);
    vklCommit(vklSampler);
  }

  ~Structured()
  {
    vklRelease(vklSampler);
    volume.reset();  // also releases the vklVolume handle
  }

  inline VKLVolume getVolume() const
  {
    return vklVolume;
  }

  inline VKLSampler getSampler() const
  {
    return vklSampler;
  }

  std::unique_ptr<WaveletStructuredRegularVolume<float>> volume;
  VKLVolume vklVolume{nullptr};
  VKLSampler vklSampler{nullptr};
};


// based on BENCHMARK_MAIN() macro from benchmark.h
int main(int argc, char **argv)
{
  vklLoadModule("ispc_driver");

  VKLDriver driver = vklNewDriver("ispc");
  vklCommitDriver(driver);
  vklSetCurrentDriver(driver);

  registerVolumeBenchmarks<Structured>();

  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;

  ::benchmark::RunSpecifiedBenchmarks();

  vklShutdown();

  return 0;
}
