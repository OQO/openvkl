// Copyright 2019-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "iterator/GridAcceleratorIterator.ih"
#include "iterator/Iterator.ih"
#include "math/box.ih"
#include "math/vec.ih"
#include "openvkl/openvkl.isph"

export void delete_uniform(void *uniform ptr)
{
  delete ptr;
}

export uniform int sizeofVaryingRange1f()
{
  return sizeof(varying box1f);
}

export varying box1f *uniform newVaryingRange1f()
{
  return uniform new varying box1f;
}

export uniform int sizeofVaryingVec3f()
{
  return sizeof(varying vec3f);
}

export varying vec3f *uniform newVaryingVec3f()
{
  return uniform new varying vec3f;
}

export uniform int sizeofVaryingVKLIntervalIterator()
{
  return sizeof(varying VKLIntervalIterator);
}

export varying Interval *uniform newVaryingInterval()
{
  return uniform new varying Interval;
}

export uniform int sizeofVaryingInterval()
{
  return sizeof(varying Interval);
}

export uniform int sizeofVaryingHit()
{
  return sizeof(varying Hit);
}

export varying Hit *uniform newVaryingHit()
{
  return uniform new varying Hit;
}

export uniform int sizeofGridAcceleratorIteratorU()
{
  return sizeof(uniform GridAcceleratorIterator);
}

export uniform int sizeofGridAcceleratorIteratorV()
{
  return sizeof(varying GridAcceleratorIterator);
}

export void *uniform newGridAcceleratorIteratorU()
{
  return uniform new uniform GridAcceleratorIterator;
}

export void *uniform newGridAcceleratorIteratorV()
{
  return uniform new varying GridAcceleratorIterator;
}
