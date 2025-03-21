/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
// testcase #issue 1048
#include "vaip/vaip.hpp"
#include "vitis/ai/env_config.hpp"
#include <glog/logging.h>
DEF_ENV_PARAM(DEBUG_CONVERT_ENDING_BLACKLIST_OPS_TO_UNKNOWN_OP, "0")
#define MY_LOG(n)                                                              \
  LOG_IF(INFO, ENV_PARAM(DEBUG_CONVERT_ENDING_BLACKLIST_OPS_TO_UNKNOWN_OP) >= n)
namespace {

using namespace vaip_core;

static bool is_black_list_op(const Node& node) {
  auto ret = false;
  for (auto s : std::vector<std::string>{"Flatten", "Reshape", "Squeeze"}) {
    ret = ret || vaip_core::node_is_op(node, s, "");
  }
  return ret;
}

struct ConvertEndingBlacklistOpsToUnknownOp {
  void preprocess(IPass& self, Graph& graph) {
    auto leaf_nodes = graph_get_output_nodes(graph);
    std::vector<const Node*> black_list_leaf_nodes;
    for (auto n : leaf_nodes) {
      if (is_black_list_op(*n)) {
        black_list_leaf_nodes.push_back(n);
      }
    }
    if (!black_list_leaf_nodes.empty()) {
      VAIP_ORT_API(graph_reverse_dfs_from)
      (
          graph,                                             //
          black_list_leaf_nodes,
          nullptr,                                           //
          [&](const Node* node) { nodes_.push_back(node); }, //
          // from black_list_leaf_nodes to the following (return ture)'s "from
          // Node" will be added to nodes_
          [&](const Node* from, const Node* to) {
            return is_black_list_op(*from) && !is_black_list_op(*to);
          });
    }
  }
  bool process(IPass& self, Graph& graph) {
    auto ret = false;
    for (auto node : nodes_) {
      if (node_is_op(*node, "const", "com.xilinx")) {
        continue;
      }
      auto outputs = node_get_output_node_args(*node);
      if (outputs.size() > 1) {
        continue;
      }
      if (node_arg_is_unknown_shape(*outputs[0])) {
        continue;
      }
      MY_LOG(1) << "to be change to unknown op : " << node_as_string(*node);
      auto shape = node_get_output_shape(*node, 0);
      NodeBuilder(graph, self)
          .clone_inputs(*node)
          .set_op_type("unknown")
          .clone_data_type(*node)
          .set_shape(shape)
          .set_anchor_point1(*node)
          .build();
      ret = true;
    }
    return ret;
  }

  static std::unique_ptr<Rule> create_rule() {
    auto builder = PatternBuilder();
    std::shared_ptr<Pattern> pat_xxx = builder.wildcard();
    return Rule::create_rule(pat_xxx,
                             [=](onnxruntime::Graph* graph,
                                 binder_t& binder) -> bool { return false; });
  }
  ConvertEndingBlacklistOpsToUnknownOp(IPass& self) : self_{self} {}

public:
  IPass& self_;
  std::vector<const Node*> nodes_;
};
} // namespace
DEFINE_VAIP_PASS(ConvertEndingBlacklistOpsToUnknownOp,
                 vaip_pass_convert_ending_blacklist_ops_to_unknown_op)
