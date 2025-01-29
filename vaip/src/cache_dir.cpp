/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#include "./cache_dir.hpp"

#include <filesystem>
#include <glog/logging.h>
#include <vitis/ai/env_config.hpp>
DEF_ENV_PARAM_2(USERNAME, "", std::string)
DEF_ENV_PARAM_2(USER, "", std::string)
DEF_ENV_PARAM_2(XLNX_CACHE_DIR, "", std::string)

namespace fs = std::filesystem;
namespace vaip_core {
static std::string get_user_name() {
  auto ret = std::string();
  if (!ENV_PARAM(USERNAME).empty()) {
    ret = ENV_PARAM(USERNAME);
  } else if (!ENV_PARAM(USER).empty()) {
    ret = ENV_PARAM(USER);
  }
  return ret;
}

static fs::path default_cache_directory() {
  auto tmp_dir =
#ifdef _WIN32
      fs::path("C:\\temp");
#else
      fs::path("/tmp");
#endif
  return tmp_dir / get_user_name() / "vaip" / ".cache";
}

bool file_exists(const fs::path& filename) { return fs::exists(filename); }

fs::path get_cache_file_name(const PassContext& context,
                             const std::string& filename) {
  auto cache_dir = context.get_log_dir();
  return cache_dir / filename;
}

void update_cache_dir(PassContextImp& context) {
  auto cache_dir = fs::path(ENV_PARAM(XLNX_CACHE_DIR));
  // use json config first.
  auto config_cache_dir = context.context_proto.config().cache_dir();
  if (!config_cache_dir.empty()) {
    cache_dir = fs::path(config_cache_dir);
  }
  if (ENV_PARAM(XLNX_CACHE_DIR).empty() && config_cache_dir.empty()) {
    cache_dir = default_cache_directory();
  }
  context.log_dir = cache_dir / context.context_proto.config().cache_key();
  *context.context_proto.mutable_config()->mutable_cache_dir() =
      cache_dir.string();
  if (context.cache_in_mem()) {
    LOG(WARNING) << "skip update cache dir: in-mem mode";
    return;
  }
  if (!fs::exists(context.log_dir) &&
      !fs::create_directories(context.log_dir)) {
    LOG(WARNING) << "cannot create cache directory: dir=" << context.log_dir;
  }
}

} // namespace vaip_core
