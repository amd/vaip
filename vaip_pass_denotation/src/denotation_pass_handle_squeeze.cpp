/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#include <glog/logging.h>

#include <algorithm>
#include <unordered_map>

#include "./denotation_pass2.hpp"
#include "vaip/vaip.hpp"
#include "vitis/ai/env_config.hpp"
DEF_ENV_PARAM(DEBUG_DENOTATION_PASS, "0")
#define MY_LOG(n) LOG_IF(INFO, ENV_PARAM(DEBUG_DENOTATION_PASS) >= n)
namespace vaip_pass_denotation {

void NodeActionState::handle_squeeze(const Graph& graph, const Node& node) {
  auto input_layout = this->input_layouts_[0].get();
  auto output_layout = this->output_layouts_[0].get();
  if (nullptr == input_layout || nullptr == output_layout) {
    return;
  }
  auto input_shape_ptr = node_arg_get_shape_i64(*inputs_[0]);
  auto output_shape_ptr = node_arg_get_shape_i64(*outputs_[0]);
  if (nullptr == input_shape_ptr || nullptr == output_shape_ptr) {
    return;
  }
  auto input_shape = *input_shape_ptr;
  auto output_shape = *output_shape_ptr;
  CHECK_EQ(input_shape.size(), input_layout->size()) << node_as_string(node);
  CHECK_EQ(output_shape.size(), output_layout->size()) << node_as_string(node);
  auto axes = std::vector<int64_t>();
  // if Squeeze op_ver=11, it has "axes" attr, if Squeeze op_ver=13, "axes" move
  // to input[1]. test case: model 40, OFA-depthwise-res50.
  if (node_has_attr(node, "axes")) {
    auto axes_ori = node_get_attr_ints(node, "axes");
    axes.assign(axes_ori.begin(), axes_ori.end());
    for (auto& a : axes) {
      if (a < 0) {
        a = ((int64_t)input_shape.size()) + a;
      }
    }
  } else if (this->input_layouts_.size() == 1) {
    axes.reserve(input_shape.size());
    for (auto i = 0u; i < input_shape.size(); ++i) {
      if (input_shape[i] == 1) {
        axes.push_back((int64_t)i);
      }
    }
  } else {
    auto inputs = node_get_inputs(node);
    CHECK_GE(inputs.size(), 2u) << node_as_string(node);
    auto ni = inputs[1];
    gsl::span<const int64_t> span;
    if (ni.node == nullptr) {
      // test case 112
      span = node_arg_get_const_data_as_i64s(graph, *ni.node_arg);
    } else {
      // no test on this branch, only support constant second input for squeeze.
      LOG(FATAL) << "not support squeeze's second input is node";
    }
    axes.assign(span.begin(), span.end());
    for (auto& a : axes) {
      if (a < 0) {
        a = ((int64_t)input_shape.size()) + a;
      }
    }
  }
  CHECK_GE(input_shape.size(), axes.size()) << node_as_string(node);
  auto left_axes = std::vector<int64_t>(input_shape.size() - axes.size());
  CHECK_EQ(left_axes.size(), output_shape.size()) << node_as_string(node);
  auto c = 0u;
  for (auto a = 0u; a < input_shape.size(); ++a) {
    auto found = std::find(axes.begin(), axes.end(), a) != axes.end();
    if (!found) {
      left_axes[c] = a;
      c = c + 1;
    }
  }
  CHECK_EQ(c, left_axes.size()) << node_as_string(node);
  c = 0;
  for (auto a : left_axes) {
    this->negotiate_layout(std::vector<denotation_view_t>{
        {(*input_layout)[a]}, {(*output_layout)[c]}});
    c = c + 1;
  }
}
} // namespace vaip_pass_denotation
