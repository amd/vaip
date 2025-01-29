/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#pragma once
#include "./_sanity_check.hpp"
#include <cstdlib>
#include <vaip/export.h>
#include <vector>
namespace vaip_core {
VAIP_DLL_SPEC
std::vector<std::pair<std::vector<size_t>, std::vector<size_t>>>
guess_reshape(const std::vector<int64_t>& shape_1,
              const std::vector<int64_t>& shape_2);
} // namespace vaip_core
