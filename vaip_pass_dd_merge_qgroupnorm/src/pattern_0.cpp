/*
 *     The Xilinx Vitis AI Vaip in this distribution are provided under the
 * following free and permissive binary-only license, but are not provided in
 * source code form.  While the following free and permissive license is similar
 * to the BSD open source license, it is NOT the BSD open source license nor
 * other OSI-approved open source license.
 *
 *      Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc.
 *
 *      Redistribution and use in binary form only, without modification, is
 * permitted provided that the following conditions are met:
 *
 *      1. Redistributions must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 *      2. The name of Xilinx, Inc. may not be used to endorse or promote
 * products redistributed with this software without specific prior written
 * permission.
 *
 *      THIS SOFTWARE IS PROVIDED BY XILINX, INC. "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL XILINX, INC. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *      PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 */
#include <glog/logging.h>

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wpedantic"
#  pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "vaip/dd/coeffs.hpp"
#include "vaip/dd/dd_utils.hpp"
#include "vaip/pattern_zoo.hpp"

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif
#include "vaip/vaip.hpp"
#include "vitis/ai/env_config.hpp"

DEF_ENV_PARAM(DEBUG_DD_MERGE_QGROUPNORM, "0")
#define MY_LOG(n) LOG_IF(INFO, ENV_PARAM(DEBUG_DD_MERGE_QGROUPNORM) >= n)

/**
 * test case: <???>
 *
 *
 * Replace pattern:
 *
 * From: <???>
 * To  : <???>
 */

// add the following line in your vaip_config.json
/*
    { "name": "vaip_pass_dd_merge_qgroupnorm_0",
       "plugin": "vaip-pass_dd_merge_qgroupnorm_0",
       "disabled": false
    }
*/
namespace {
using namespace vaip_core;

// direct python translation, may need to change later
std::pair<std::vector<int64_t>, std::vector<int64_t>>
get_NCHW_NHWC(const std::vector<int64_t>& shapes) {
  if (shapes.size() == 4) {
    if (shapes[1] == shapes[2]) {
      return {{shapes[0], shapes[3], shapes[1], shapes[2]}, shapes};
    } else if (shapes[2] == shapes[3]) {
      return {shapes, {shapes[0], shapes[2], shapes[3], shapes[1]}};
    }
  }
  return {shapes, shapes};
}

std::tuple<float, uint16_t, std::string>
get_sibling_concat_qparams_grp_nrm_0(onnxruntime::Graph* graph,
                                     const NodeInput& in_node, float in_scale,
                                     uint16_t in_zero_point) {
  std::string concat_in_sibling = "false";
  auto node_found = in_node.node;
  if (node_found != nullptr) {
    auto op_type = VAIP_ORT_API(node_op_type)(*node_found);
    if (VAIP_ORT_API(node_op_type)(*node_found) == "IConv") {
      auto concat_attr = node_has_attr(*node_found, "concat_in_child");

      if (concat_attr &&
          node_get_attr_string(*node_found, "concat_in_child") == "true") {
        in_scale = node_get_attr_float(*node_found, "output_scale");
        in_zero_point =
            (uint16_t)(node_get_attr_float(*node_found, "output_zp"));
        concat_in_sibling = "true";

        MY_LOG(1) << "GP has concat in silbling";
      }
    } else if (VAIP_ORT_API(node_op_type)(*node_found) ==
               "QuantizeLinear") { // if producer node is QuantizeLinear
      // node_found
      auto quant_node_name =
          node_get_first_output_name(*node_found); // input or output
      auto quant_consumers = graph_get_consumer_nodes(*graph, quant_node_name);
      for (auto consumer :
           quant_consumers) { // for each consumer of QuantizeLinear
        if (VAIP_ORT_API(node_op_type)(*consumer) == "DequantizeLinear") {
          auto dequant_node_name = node_get_first_output_name(*consumer);
          auto dequant_consumers =
              graph_get_consumer_nodes(*graph, dequant_node_name);
          if ((VAIP_ORT_API(node_op_type)(*dequant_consumers[0]) ==
               "Concat")) { // Check if Concat is in the consumer of
                            // QuantLinear
            auto concat_node_name =
                node_get_first_output_name(*dequant_consumers[0]);
            auto concat_consumers =
                graph_get_consumer_nodes(*graph, concat_node_name);
            auto Q_node_input_node_args =
                node_get_input_node_args(*concat_consumers[0]);
            in_scale = node_arg_get_const_data_as_float(
                *graph, *Q_node_input_node_args[1]);
            in_zero_point =
                vaip::dd::get_zp_from_node(*graph, *Q_node_input_node_args[2]);
            concat_in_sibling = "true";
            MY_LOG(1) << "GP node with concat sibling updated here ";
          }
        }
      }
    }
  }
  return std::make_tuple(in_scale, in_zero_point, concat_in_sibling);
}

struct Dd_merge_qgroupnorm_0 {
  Dd_merge_qgroupnorm_0(IPass& self) : self_{self} {}
  std::unique_ptr<Rule> create_rule(IPass* self) {
    auto com_microsoft_QuantizeLinear_5 =
        vaip::pattern_zoo::get_pattern("m_qgroupnorm_0");
    CHECK(com_microsoft_QuantizeLinear_5 != nullptr)
        << "Pattern returned is null";

    return Rule::create_rule(
        com_microsoft_QuantizeLinear_5,
        [=](onnxruntime::Graph* graph, binder_t& binder) -> bool {
          // this is the python pattern "QGrpNormTrans":
          // QGroupNorm pattern(pattern 1) + Transpose
          // must run before pattern 1
          auto in_node = binder["input_0"];
          auto in_scale_node = binder["constant_12"];
          auto in_zp_node = binder["constant_13"];
          auto out_scale_node = binder["constant_20"];
          auto out_zp_node = binder["constant_21"];
          auto instnorm_node = binder["InstanceNormalization_0"];
          auto mul_const_in_node = binder["constant_3"];
          auto mul_const_scale_node = binder["constant_4"];
          auto mul_const_zp_node = binder["constant_5"];
          auto add_const_in_node = binder["constant_0"];
          auto add_const_scale_node = binder["constant_1"];
          auto add_const_zp_node = binder["constant_2"];
          auto out_node = binder["com_microsoft_QuantizeLinear_5"];

          std::vector<std::string> ns = vaip::dd::get_node_names(graph, binder);
          MY_LOG(1) << "found match at "
                    << node_arg_get_name(*out_node.node_arg);

          // attrs
          auto epsilon = node_get_attr_float(*instnorm_node.node, "epsilon");

          // in/out shape
          auto in_shape = node_arg_get_shape_i64(*in_node.node_arg);
          auto [nchw_shape, nhwc_shape] = get_NCHW_NHWC(*in_shape);
          std::vector<int64_t> new_out_shape{1, nhwc_shape[1] * nhwc_shape[2],
                                             nhwc_shape[3]};

          // constants
          auto in_scale =
              node_arg_get_const_data_as_float(*graph, *in_scale_node.node_arg);
          auto in_zero_point =
              node_arg_get_const_data_as_u16(*graph, *in_zp_node.node_arg);

          // update input q params if concat is sibling op ( 2 cases parent can
          // be Iconv or QEltwiseAdd)

          std::string concat_in_sibling = "false";
          auto sibling_concat_params = get_sibling_concat_qparams_grp_nrm_0(
              graph, in_node, in_scale, in_zero_point);
          if (std::get<2>(sibling_concat_params) == "true") {
            in_scale = std::get<0>(sibling_concat_params);
            in_zero_point = std::get<1>(sibling_concat_params);
            concat_in_sibling = std::get<2>(sibling_concat_params);
          }

          std::vector<float> input_q_params{in_scale, float(in_zero_point)};
          auto out_scale = node_arg_get_const_data_as_float(
              *graph, *out_scale_node.node_arg);
          auto out_zero_point =
              node_arg_get_const_data_as_u16(*graph, *out_zp_node.node_arg);
          std::vector<float> output_q_params{out_scale, float(out_zero_point)};

          auto node_name = node_arg_get_name(*out_node.node_arg);
          // gamma
          auto mul_const_in = node_arg_get_const_data_as_u16s(
              *graph, *mul_const_in_node.node_arg);
          std::vector<uint16_t> mul_const_vec(mul_const_in.begin(),
                                              mul_const_in.end());
          auto mul_const_scale = node_arg_get_const_data_as_float(
              *graph, *mul_const_scale_node.node_arg);
          auto mul_const_zp = node_arg_get_const_data_as_u16(
              *graph, *mul_const_zp_node.node_arg);
          auto gamma = vaip::dd::qmatmulcalc::dq_vec_to_bf16(
              mul_const_vec, mul_const_scale, mul_const_zp);
          auto gamma_shape =
              node_arg_get_shape_i64(*mul_const_in_node.node_arg);
          auto& input_gamma_arg = vaip::dd::insert_named_tensor_in_graph(
              graph, node_name + "_gamma_", gamma, *gamma_shape);

          // beta
          auto add_const_in = node_arg_get_const_data_as_u16s(
              *graph, *add_const_in_node.node_arg);
          std::vector<uint16_t> add_const_vec(add_const_in.begin(),
                                              add_const_in.end());
          auto add_const_scale = node_arg_get_const_data_as_float(
              *graph, *add_const_scale_node.node_arg);
          auto add_const_zp = node_arg_get_const_data_as_u16(
              *graph, *add_const_zp_node.node_arg);
          auto beta = vaip::dd::qmatmulcalc::dq_vec_to_bf16(
              add_const_vec, add_const_scale, add_const_zp);
          auto beta_shape = node_arg_get_shape_i64(*add_const_in_node.node_arg);
          auto& input_beta_arg = vaip::dd::insert_named_tensor_in_graph(
              graph, node_name + "_beta_", beta, *beta_shape);

          // qdq
          std::vector<int32_t> input_qdq(16, 0);
          std::tie(input_qdq[0], input_qdq[1]) =
              vaip::dd::qmatmulcalc::calc_lrn_coeff(1 / out_scale,
                                                    out_zero_point);
          std::tie(input_qdq[3], input_qdq[4]) =
              vaip::dd::qmatmulcalc::calc_lrn_coeff(in_scale, in_zero_point);
          // input_qdq[2] and input_qdq[5] depend on input/ouput tensor type
          // hardcoded for now, may need change
          input_qdq[2] = 1;
          input_qdq[5] = 1;
          auto& input_qdq_arg = vaip::dd::insert_named_tensor_in_graph(
              graph, node_name + "_qdq_", input_qdq,
              std::vector({(int64_t)input_qdq.size()}));

          // hardcode for mzdk5, may need change
          std::vector<std::string> input_types{"uint16", "uint16", "uint16",
                                               "int32"};
          std::vector<std::string> output_types{"uint16"};

          NodeBuilder(*graph, *self)
              .set_input_node_args({in_node.node_arg, &input_gamma_arg,
                                    &input_beta_arg, &input_qdq_arg})
              .set_op_type("QGroupNorm", "com.xilinx")
              .add("nodes", ns)
              .add("epsilon", epsilon)
              .add("in_dtypes", input_types)
              .add("out_dtypes", output_types)
              .add("input_q_params", input_q_params)
              .add("output_q_params", output_q_params)
              .add("input_shape", nhwc_shape)
              .add("output_shape", new_out_shape)
              .add("concat_in_sibling",
                   concat_in_sibling) // used by ops that are consumers of
                                      // IConv, if concat is one of the
                                      // consumers of IConv
              .add("input_scale", in_scale)
              .add("input_zp", (float)in_zero_point)
              .set_anchor_point1(*out_node.node)
              .build();
          return true;
        });
  }
  // apply the rule
  void process(IPass& self, Graph& graph) {
    MY_LOG(1) << self_.get_pass_proto().name() << "["
              << self_.get_pass_proto().plugin() << "] start processing graph";
    create_rule(&self)->apply(&graph);
    MY_LOG(1) << self_.get_pass_proto().name() << "["
              << self_.get_pass_proto().plugin() << "] finish processing graph";
  }

  IPass& self_;
};
} // namespace

DEFINE_VAIP_PASS(Dd_merge_qgroupnorm_0, vaip_pass_dd_merge_qgroupnorm_0)