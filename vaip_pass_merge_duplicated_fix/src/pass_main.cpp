/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
// test case 41 see issue #611 #626 for more detail
#include "vaip/vaip.hpp"
#include "vitis/ai/env_config.hpp"
#include <glog/logging.h>
DEF_ENV_PARAM(DEBUG_MERGE_DUPLICATED_FIX, "0")
#define MY_LOG(n) LOG_IF(INFO, ENV_PARAM(DEBUG_MERGE_DUPLICATED_FIX) >= n)
namespace {
using namespace vaip_core;
struct MergeDuplicatedFix {
  MergeDuplicatedFix(IPass& self) : self_{self} {}
  void preprocess(IPass& self, Graph& graph) {
    auto nodes = graph_nodes(graph);
    for (auto& node : nodes) {
      auto node_args = node_get_output_node_args(*node);
      if (node_args.size() == 1) {
        auto consumers =
            graph_get_consumer_nodes(graph, node_arg_get_name(*node_args[0]));
        auto num_of_consumers = consumers.size();
        if (num_of_consumers >= 2) {
          const auto& first_consumer = *consumers[0];
          auto is_first_consumer_fix_op =
              node_is_op(first_consumer, "fix", "com.xilinx");
          if (is_first_consumer_fix_op) {
            auto fix_point = node_get_attr_int(first_consumer, "fix_point");
            auto all_consumer_are_fix = true;
            for (auto i = 1u; i < num_of_consumers; ++i) {
              const auto& rest_consumer = *consumers[i];
              all_consumer_are_fix =
                  all_consumer_are_fix &&
                  node_is_op(rest_consumer, "fix", "com.xilinx");
              all_consumer_are_fix =
                  all_consumer_are_fix &&
                  fix_point == node_get_attr_int(rest_consumer, "fix_point");
            }
            if (all_consumer_are_fix) {
              common_fix_nodes_.emplace_back(consumers);
            }
          }
        }
      }
    }
  }

  void process(const IPass& self, Graph& graph) {
    for (auto& m : common_fix_nodes_) {
      const auto& first_fix_node = *m[0];
      for (auto i = 1u; i < m.size(); ++i) {
        const auto& first_node_arg = node_get_output_node_arg(first_fix_node);
        MY_LOG(1) << "replace "
                  << node_arg_get_name(node_get_output_node_arg(*m[i]))
                  << " with " << node_arg_get_name(first_node_arg);
        graph_replace_node_arg(graph, self_,
                               node_get_output_node_arg(*m[i]) /*from*/,
                               node_get_output_node_arg(first_fix_node) /*to*/);
      }
    }
  }

  IPass& self_;
  std::vector<std::vector<const Node*>> common_fix_nodes_;
};
} // namespace
DEFINE_VAIP_PASS(MergeDuplicatedFix, vaip_pass_merge_duplicated_fix)
