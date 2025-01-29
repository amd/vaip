/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
#pragma once
#include "vaip/node.hpp"
#include "vaip/node_arg.hpp"
#include "vitis/ai/env_config.hpp"
#include <glog/logging.h>
DEF_ENV_PARAM(DEBUG_VAIP_PATTERN, "0")
#define MY_LOG(n) LOG_IF(INFO, ENV_PARAM(DEBUG_VAIP_PATTERN) >= n)
#define MATCH_FAILED MY_LOG(1) << "MATCH FAILED. ID=" << get_id() << ";"
namespace vaip_core {
[[maybe_unused]] static std::string node_input_as_string(const NodeInput& ni) {
  if (ni.node) {
    return node_as_string(*ni.node);
  } else if (ni.node_arg) {
    return node_arg_as_string(*ni.node_arg);
  }
  return "nil";
}
} // namespace vaip_core
