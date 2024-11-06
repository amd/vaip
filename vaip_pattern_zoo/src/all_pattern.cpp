/*
# Copyright (C) 2022 Xilinx Inc.
 *      Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc.
 *
 *      Redistribution and use in binary form only, without modification, is
 * permitted provided that the following conditions are met:
 *
 *      1. Redistributions must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 *      2. The name of Xilinx, Inc. may not be used to endorse or promote
 * products redistributed with this software without specific prior written
 * permission.
 *
 *      THIS SOFTWARE IS PROVIDED BY XILINX, INC. "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL XILINX, INC. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *      PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 */

#include "vaip/pattern_zoo.hpp"
#include <functional>
#include <glog/logging.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace vaip {
namespace pattern_zoo {
VAIP_DLL_SPEC std::vector<std::string> pattern_list() {
  auto patterns = std::vector<std::string>();
  VAIP_ORT_API(vaip_get_pattern_list)
  (reinterpret_cast<void*>(&patterns), [](void* env, void* data, size_t size) {
    auto ret = reinterpret_cast<std::vector<std::string>*>(env);
    ret->emplace_back((const char*)data, size);
  });
  return patterns;
}
VAIP_DLL_SPEC std::shared_ptr<vaip_core::Pattern>
get_pattern(const std::string& name) {

  std::shared_ptr<vaip_core::Pattern> pattern;
  auto ret = VAIP_ORT_API(vaip_get_pattern_as_binary)(
      name.data(), reinterpret_cast<void*>(&pattern),
      [](void* env, void* data, size_t size) {
        auto ret = reinterpret_cast<std::shared_ptr<vaip_core::Pattern>*>(env);
        *ret = vaip_core::PatternBuilder().create_from_binary((const char*)data,
                                                              size);
      });
  if (ret != 0) {
    LOG(WARNING) << "Dose not exist pattern : " << name;
    pattern = nullptr;
  }
  return pattern;
}
} // namespace pattern_zoo
} // namespace vaip
