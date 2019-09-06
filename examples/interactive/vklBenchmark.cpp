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

#include "AppInit.h"
#include "window/VKLWindow.h"
// openvkl_testing
#include "openvkl_testing.h"
// google benchmark
#include "benchmark/benchmark.h"
// std
#include <sstream>
// ospcommon
#include "ospcommon/common.h"
#include "ospcommon/math/box.h"

using namespace openvkl::examples;
using namespace openvkl::testing;
using namespace ospcommon::math;

static void volume_render_wavelet(benchmark::State &state,
                                  const std::string &rendererType,
                                  const vec2i &windowSize,
                                  int volumeDimension)
{
  auto proceduralVolume = ospcommon::make_unique<WaveletProceduralVolume>(
      vec3i(volumeDimension), vec3f(-1.f), vec3f(2.f / volumeDimension));

  VKLVolume volume    = proceduralVolume->getVKLVolume();
  VKLSamplesMask mask = vklNewSamplesMask(volume);
  auto bounds         = vklGetBoundingBox(volume);
  auto voxelRange     = proceduralVolume->getVoxelRange();

  auto ow = ospcommon::make_unique<VKLWindow>(
      windowSize, (box3f &)bounds, volume, voxelRange, mask, rendererType);

  for (auto _ : state) {
    ow->render();
  }

  // enables rates in report output
  state.SetItemsProcessed(state.iterations());

  // save image on completion of benchmark; note we apparently have no way to
  // get the formal benchmark name, so we'll create one here
  static int ppmCounter = 0;
  std::stringstream ss;
  ss << "volume_render_wavelet_" << ppmCounter << ".ppm";
  ow->savePPM(ss.str());
  ppmCounter++;
}

BENCHMARK_CAPTURE(volume_render_wavelet,
                  simple_native / 512,
                  "simple_native",
                  vec2i(1024),
                  512);

BENCHMARK_CAPTURE(
    volume_render_wavelet, simple_vkl / 512, "simple_vkl", vec2i(1024), 512);

BENCHMARK_CAPTURE(volume_render_wavelet,
                  vkl_interval_iterator / 512,
                  "vkl_interval_iterator",
                  vec2i(1024),
                  512);

BENCHMARK_CAPTURE(volume_render_wavelet,
                  vkl_iterator / 512,
                  "vkl_iterator",
                  vec2i(1024),
                  512);

// based on BENCHMARK_MAIN() macro from benchmark.h
int main(int argc, char **argv)
{
  initializeOpenVKL();

  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();

  vklShutdown();
}