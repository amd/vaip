/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#include "vaip/util.hpp"
#include <dlfcn.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace vaip_core {
VAIP_DLL_SPEC std::filesystem::path get_vaip_path() {
  Dl_info info;
  if (dladdr(reinterpret_cast<const void*>(&get_vaip_path), &info)) {
    return std::filesystem::path(info.dli_fname);
  }
  return {};
}
unsigned int get_tid() {
  return static_cast<unsigned int>(syscall(SYS_gettid));
}

unsigned int get_pid() {
  return static_cast<unsigned int>(syscall(SYS_getpid));
}

} // namespace vaip_core
