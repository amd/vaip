/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#include "./pattern_node.hpp"

#include <glog/logging.h>

#include "./pattern_constant.hpp"
#include "node_arg.hpp"
#include "vaip/node.hpp"
#include "vaip/util.hpp"
#include <vaip/vaip_ort_api.h>

#include "./pattern_log.hpp"
#include "vaip/pattern.pb.h"

namespace vaip_core {

std::string get_op_type(const std::string& op_type) {
  auto pos = op_type.find(':');
  if (pos == std::string::npos) {
    return std::string("onnx") + ":" + op_type;
  }
  return op_type;
}
// op_type is in format `domain::op_type`

PatternNode::PatternNode(int id, const std::string& op_type,
                         std::vector<std::shared_ptr<Pattern>> args,
                         std::vector<bool> is_args_optional)
    : Pattern(id), op_type_(get_op_type(op_type)), args_(std::move(args)),
      is_args_optional_(std::move(is_args_optional)) {
  CHECK(args_.size() == is_args_optional_.size());
}

PatternNode::~PatternNode() {}
// 1: op_type must be equal
// 2: input_pattern match all
static std::string get_full_op_type(const onnxruntime::Node& node) {
  auto domain = VAIP_ORT_API(node_op_domain)(node);
  if ("" == domain) {
    domain = "onnx";
  };
  return domain + ":" + VAIP_ORT_API(node_op_type)(node);
}

BinderBuilderPtr
PatternNode::match_uncached(const onnxruntime::Graph& graph,
                            const NodeInput& node_input,
                            const BinderBuilder& binder) const {
  if (node_input.node == nullptr) {
    MATCH_FAILED << " not a node: " << node_arg_as_string(*node_input.node_arg);
    return nullptr;
  }
  auto& node = *node_input.node;
  auto full_op_type = get_full_op_type(node);
  if (full_op_type != this->op_type_) {
    MATCH_FAILED << " expect node_type is " << this->op_type_
                 << " actually node type is " << full_op_type
                 << node_as_string(node);
    return nullptr;
  }
  auto inputs = node_get_inputs(node);
  auto inputs_size = inputs.size();
  auto args_size = args_.size();
  // you can not match a node with three inputs but two arguments
  if (inputs_size > args_size) {
    MATCH_FAILED << " too many inputs. expect num of args is " << args_size
                 << " actual input size  is " << inputs_size
                 << "; node=" << node_as_string(node);
    return nullptr;
  }
  auto ret = binder.add(this->get_id(), node_input);
  for (auto arg_idx = 0u; arg_idx < inputs_size; ++arg_idx) {
    if (is_args_optional_[arg_idx]) {
      if (node_arg_exists(*inputs[arg_idx].node_arg)) {
        ret = args_[arg_idx]->match_cached(graph, inputs[arg_idx], *ret);
      } else {
        // it is ok if the arg does not exits, because it is optional.
      }
    } else {
      ret = args_[arg_idx]->match_cached(graph, inputs[arg_idx], *ret);
    }
    if (ret == nullptr) {
      MATCH_FAILED << " arg[" << arg_idx
                   << "] is not match, pattern_id=" << args_[arg_idx]->get_id();
      return nullptr;
    }
  }
  for (auto arg_idx = inputs_size; arg_idx < args_size; ++arg_idx) {
    if (!is_args_optional_[arg_idx]) {
      MATCH_FAILED << " arg[" << arg_idx
                   << "] is required. args_size=" << args_size;
      return nullptr;
    }
  }
  MY_LOG(1) << "MATCH OK. ID=" << get_id() << ", node=" << node_as_string(node);
  return ret;
}

std::string PatternNode::debug_string() const {
  auto ret = std::string("#");
  ret += std::to_string(this->get_id()) + std::string("(");
  ret += this->op_type_;
  if (!args_.empty()) {
    ret += std::string("(");
    for (auto i = 0u; i < args_.size() - 1; i++) {
      ret += args_[i]->debug_string() + ", ";
    }
    ret += args_[args_.size() - 1]->debug_string();
    ret += std::string(")");
  }
  ret += std::string(")");
  return ret;
}
void PatternNode::dump_to_proto_imp(RootPatternProto& pattern_proto,
                                    PatternProto& this_proto) const {
  auto proto = this_proto.mutable_call_node();
  LOG_IF(INFO, ENV_PARAM(DEBUG_VAIP_PATTERN))
      << " pattern: " << this->get_id() << " ";
  proto->set_op_type(this->op_type_);
  for (auto is_optional : is_args_optional_) {
    proto->add_optional_args(is_optional);
  }
  for (auto arg : args_) {
    auto arg_pattern_proto = arg->dump_to_proto(pattern_proto);
    LOG_IF(INFO, ENV_PARAM(DEBUG_VAIP_PATTERN))
        << " pattern arg: " << arg->get_id() << " ";
    proto->add_args()->set_id(arg_pattern_proto->id());
  }
}
} // namespace vaip_core
