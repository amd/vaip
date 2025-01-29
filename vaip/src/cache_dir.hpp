/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#pragma once
#include "./pass_imp.hpp"
#include <filesystem>
#include <string>
namespace vaip_core {
bool file_exists(const std::filesystem::path& filename);
// return a cache directory.
std::filesystem::path get_cache_file_name(const PassContext& context,
                                          const std::string& filename);
void update_cache_dir(PassContextImp& context);
} // namespace vaip_core
