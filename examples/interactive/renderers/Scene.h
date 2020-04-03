// Copyright 2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#if defined(ISPC)

#include "openvkl/openvkl.isph"

#endif // defined(ISPC)

#if defined(__cplusplus)

#include "openvkl/openvkl.h"
#include "TransferFunction.h"

namespace openvkl {
  namespace examples {

#endif // defined(__cplusplus)

/*
 * This object stores all scene data. Renderers
 * read this to produce pixels.
 */
struct Scene
{
  /*
   * Our examples display a single volume.
   */
  VKLVolume        volume;
  VKLValueSelector valueSelector;

  /*
   * Shading is done through a transfer function.
   */
  box1f        tfValueRange;
  unsigned int tfNumColorsAndOpacities;
  const vec4f* tfColorsAndOpacities;

#if defined(__cplusplus)
  Scene()
    : volume(nullptr),
      valueSelector(nullptr),
      tfNumColorsAndOpacities(0),
      tfColorsAndOpacities(nullptr)
  {
  }

  ~Scene()
  {
    if (valueSelector) {
      vklRelease(valueSelector);
    }
  }

  void updateValueSelector(const TransferFunction& transferFunction,
                           const std::vector<float>& isoValues)
  {
    if (valueSelector) {
      vklRelease(valueSelector);
      valueSelector = nullptr;
    }

    if (!volume)
      return;

    valueSelector = vklNewValueSelector(volume);

      // set value selector value ranges based on transfer function positive
      // opacity intervals
      std::vector<range1f> valueRanges =
          transferFunction.getPositiveOpacityValueRanges();

      vklValueSelectorSetRanges(valueSelector,
                                valueRanges.size(),
                                (const vkl_range1f *)valueRanges.data());

      // if we have isovalues, set these values on the value selector
      if (!isoValues.empty()) {
        vklValueSelectorSetValues(
            valueSelector, isoValues.size(), isoValues.data());
      }

      vklCommit(valueSelector);
  }
#endif // defined(__cplusplus)
};

#if defined(__cplusplus)

  } // namespace examples
} // namespace openvkl

#endif // defined(__cplusplus)

