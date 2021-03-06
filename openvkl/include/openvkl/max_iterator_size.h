// Copyright 2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Maximum iterator size over all supported volume and driver
// types, and for each target SIMD width.
#define VKL_MAX_INTERVAL_ITERATOR_SIZE_4 1535
#define VKL_MAX_INTERVAL_ITERATOR_SIZE_8 3071
#define VKL_MAX_INTERVAL_ITERATOR_SIZE_16 6143

#if defined(TARGET_WIDTH) && (TARGET_WIDTH == 4)
  #define VKL_MAX_INTERVAL_ITERATOR_SIZE VKL_MAX_INTERVAL_ITERATOR_SIZE_4
#elif defined(TARGET_WIDTH) && (TARGET_WIDTH == 8)
  #define VKL_MAX_INTERVAL_ITERATOR_SIZE VKL_MAX_INTERVAL_ITERATOR_SIZE_8
#else
  #define VKL_MAX_INTERVAL_ITERATOR_SIZE VKL_MAX_INTERVAL_ITERATOR_SIZE_16
#endif
#define VKL_MAX_HIT_ITERATOR_SIZE_4 1791
#define VKL_MAX_HIT_ITERATOR_SIZE_8 3583
#define VKL_MAX_HIT_ITERATOR_SIZE_16 7167

#if defined(TARGET_WIDTH) && (TARGET_WIDTH == 4)
  #define VKL_MAX_HIT_ITERATOR_SIZE VKL_MAX_HIT_ITERATOR_SIZE_4
#elif defined(TARGET_WIDTH) && (TARGET_WIDTH == 8)
  #define VKL_MAX_HIT_ITERATOR_SIZE VKL_MAX_HIT_ITERATOR_SIZE_8
#else
  #define VKL_MAX_HIT_ITERATOR_SIZE VKL_MAX_HIT_ITERATOR_SIZE_16
#endif
