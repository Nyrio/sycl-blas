/***************************************************************************
 *  @license
 *  Copyright (C) Codeplay Software Limited
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  For your convenience, a copy of the License has been included in this
 *  repository.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  SYCL-BLAS: BLAS implementation using SYCL
 *
 *  @filename extension_trees.h
 *
 **************************************************************************/

#ifndef SYCL_BLAS_EXTENSION_TREES_H
#define SYCL_BLAS_EXTENSION_TREES_H

#include <CL/sycl.hpp>

namespace blas {

/*!
 * TODO: more info here
 */
template <typename input_t, typename output_t, typename temp_t, int ClSize,
          typename tile_type, typename element_t, bool is_final>
class ReductionPartialRows;

// TODO: make_reduction function

}  // namespace blas

#endif  // SYCL_BLAS_EXTENSION_TREES_H
