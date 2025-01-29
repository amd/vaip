/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#pragma once
#include "vitis/ai/env_config.hpp"
#include <fstream>
#include <vaip/my_ort.h>

#ifdef _WIN32
#  pragma warning(push)
#  pragma warning(disable : 4251)
#  pragma warning(disable : 4275)
#endif
#include "vaip/config.pb.h"
#ifdef _WIN32
#  pragma warning(pop)
#endif

DEF_ENV_PARAM_2(XLNX_ONNX_EP_DL_ANALYZER_VISUALIZATION, "true", bool)
DEF_ENV_PARAM_2(XLNX_ONNX_EP_DL_ANALYZER_PROFILING, "true", bool)

namespace vaip_core {
void update_config_by_target(ConfigProto& proto, const MepConfigTable* mep);
class Config {
public:
  VAIP_DLL_SPEC
  static ConfigProto parse_from_string(const char* string);
  static void merge_config_proto(ConfigProto& config_proto,
                                 const char* json_config);
  static void add_version_info(ConfigProto& config_proto);
  static void add_version_info(ConfigProto& config_proto,
                               const std::string& package_name,
                               const std::string& commit_id,
                               const std::string& version_id);

public:
  explicit Config() = default;
  explicit Config(const std::string& file);
  Config(const Config&) = delete;
  Config(Config&&) = delete;

public:
  ~Config() = default;

public:
  const ConfigProto& config_proto() const;

private:
  ConfigProto config_proto_;
};
} // namespace vaip_core
