/*******************************************************************************
 * Copyright 2020-2021 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/
#ifndef BACKEND_GRAPH_COMPILER_CORE_SRC_COMMON_DIMENSIONS_HPP
#define BACKEND_GRAPH_COMPILER_CORE_SRC_COMMON_DIMENSIONS_HPP
#include <assert.h>
#include <stdint.h>
#include <vector>

namespace sc {
using sc_dim = int64_t;
using sc_dims = std::vector<sc_dim>;
namespace dimensions {
constexpr sc_dim dynamic = -1;
}

inline uint64_t dim2unsigned(sc_dim v) {
    assert(v >= 0);
    return v;
}

} // namespace sc

#endif
