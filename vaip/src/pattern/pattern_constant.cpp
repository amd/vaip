/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#include "./pattern_constant.hpp"
#include <sstream>
#include <vaip/vaip_ort_api.h>

#include "./pattern_log.hpp"
#include "vaip/node.hpp"
#include "vaip/node_arg.hpp"
#include "vaip/pattern.pb.h"

namespace vaip_core {
PatternConstant::PatternConstant(int id) : Pattern(id) {}
PatternConstant::~PatternConstant() {}
std::string PatternConstant::debug_string() const {
  auto ret = std::string("#");
  ret += std::to_string(this->get_id()) + std::string("(");
  ret += std::string("Constant");
  ret += std::string(")");
  return ret;
}

std::string PatternConstant::virtualize_label() const {
  std::ostringstream str;
  str << "[" << this->get_id() << "] Constant";
  return str.str();
}

BinderBuilderPtr
PatternConstant::match_uncached(const onnxruntime::Graph& graph,
                                const NodeInput& node_input,
                                const BinderBuilder& binder) const {
  auto ret = BinderBuilderPtr();
  if (node_input.node != nullptr) {
    if (VAIP_ORT_API(node_op_type)(*node_input.node) == "Constant") {
      ret = binder.add(this->get_id(), node_input);
    }
  } else {
    bool is_constant =
        VAIP_ORT_API(node_arg_is_constant)(graph, *node_input.node_arg);
    if (is_constant) {
      ret = binder.add(this->get_id(), node_input);
    }
  }
  if (ret == nullptr) {
    MATCH_FAILED << "not a constant: "
                 << (node_input.node != nullptr
                         ? node_as_string(*node_input.node)
                         : node_arg_as_string(*node_input.node_arg));
  } else {
    MY_LOG(1) << "MATCH OK. ID=" << get_id() << ", constant matched: "
              << (node_input.node != nullptr
                      ? node_as_string(*node_input.node)
                      : node_arg_as_string(*node_input.node_arg));
  }
  return ret;
}
void PatternConstant::dump_to_proto_imp(RootPatternProto& pattern_proto,
                                        PatternProto& this_proto) const {
  this_proto.mutable_constant();
}
} // namespace vaip_core
