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

#include "ValueSelector.h"

namespace openvkl {
  namespace ispc_driver {

    void ValueSelector::setRanges(
        const utility::ArrayView<const range1f> &ranges)
    {
      this->ranges.clear();

      for (const auto &r : ranges) {
        this->ranges.push_back(r);
      }
    }

    void ValueSelector::setValues(const utility::ArrayView<const float> &values)
    {
      this->values.clear();

      for (const auto &v : values) {
        this->values.push_back(v);
      }
    }

  }  // namespace ispc_driver
}  // namespace openvkl