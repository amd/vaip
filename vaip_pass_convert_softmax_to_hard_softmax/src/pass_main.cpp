/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
#include <glog/logging.h>

#include "vaip/vaip.hpp"
#include "vitis/ai/env_config.hpp"

DEF_ENV_PARAM(DEBUG_CONVERT_TO_HARD_SOFTMAX, "0")
#define MY_LOG(n) LOG_IF(INFO, ENV_PARAM(DEBUG_CONVERT_TO_HARD_SOFTMAX) >= n)

/**
 * test case model 5
 *
 * Replace pattern pass
 * X : wildcard()
 * From : batchnorm(input=*, gamma=fix(const()), beta=fix(const()),
 *                  moving_mean=const().where(all_zeros()), moving_var=const())
 *
 *     output = (input - moving_mean) /
 *             sqrt(moving_var + epsilon) * gamma + beta
 *
 * To  : scale(input, weights, bias)
 *
 *  where weights and bias are `fix(const())`
 */
namespace {
using namespace vaip_core;
struct ConvertToHardSoftmax {
  ConvertToHardSoftmax(IPass& self) : self_{self} {}
  std::unique_ptr<Rule> create_rule(IPass* self) {
    auto builder = PatternBuilder();
    std::shared_ptr<Pattern> input_ = builder.wildcard();
    std::shared_ptr<Pattern> pattern_ =
        builder.node2("com.xilinx:softmax", {input_});
    return Rule::create_rule(
        pattern_, [=](onnxruntime::Graph* graph, binder_t& binder) -> bool {
          auto input = binder[input_->get_id()];
          auto pattern = binder[pattern_->get_id()];
          CHECK(input.node != nullptr);
          CHECK(pattern.node != nullptr);
          auto& cast_1 = NodeBuilder(*graph, self_)
                             .set_op_type("cast")
                             .set_input_nodes({input.node})
                             .set_data_type("bfloat16")
                             .clone_shape(*pattern.node)
                             .set_anchor_point2(*pattern.node_arg, {"cast"})
                             .build();

          auto& hsoftmax =
              NodeBuilder(*graph, self_)
                  .set_op_type("hard_softmax")
                  .set_input_nodes({&cast_1})
                  .set_data_type("bfloat16")
                  .clone_shape(*pattern.node)
                  .add("axis", node_get_attr_int(*pattern.node, "axis"))
                  .add("type", "poly")
                  .set_anchor_point2(*pattern.node_arg, {"cast"})
                  .build();
          //.clone_attrs(*pattern.node)

          NodeBuilder(*graph, self_)
              .set_op_type("cast")
              .set_input_nodes({&hsoftmax})
              .set_data_type("float32")
              .clone_shape(*pattern.node)
              .set_anchor_point1(*pattern.node)
              .build();
          return true;
        });
  }
  void process(IPass& self, Graph& graph) { create_rule(&self)->apply(&graph); }

  IPass& self_;
};
} // namespace

DEFINE_VAIP_PASS(ConvertToHardSoftmax,
                 vaip_pass_convert_softmax_to_hard_softmax)
