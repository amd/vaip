/*
 *     The Xilinx Vitis AI Vaip in this distribution are provided under the
 * following free and permissive binary-only license, but are not provided in
 * source code form.  While the following free and permissive license is similar
 * to the BSD open source license, it is NOT the BSD open source license nor
 * other OSI-approved open source license.
 *
 *      Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights
 * reserved.
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
// #include "./get_merged_attributes.hpp"

// #include "qmha/qmha_processor.hpp"
#include "vaip/dd/coeffs.hpp"
#include "vaip/dd/dd_utils.hpp"
#include "vaip/pattern_zoo.hpp"
#include "vaip/vaip.hpp"
#include "vitis/ai/env_config.hpp"
#include <glog/logging.h>
DEF_ENV_PARAM(DEBUG_DD_MERGE_QL2NORM, "0")
#define MY_LOG(n) LOG_IF(INFO, ENV_PARAM(DEBUG_DD_MERGE_QL2NORM) >= n)

namespace {
using namespace vaip_core;

struct MergeQL2norm {
  MergeQL2norm(IPass& self) : self_{self} {}
  std::unique_ptr<Rule> create_rule(IPass* self) {
    auto com_microsoft_DequantizeLinear_6 =
        vaip::pattern_zoo::get_pattern("m_ql2norm");
    CHECK(com_microsoft_DequantizeLinear_6 != nullptr)
        << "Pattern returned is null";
    return Rule::create_rule(
        com_microsoft_DequantizeLinear_6,
        [=](onnxruntime::Graph* graph, binder_t& binder) -> bool {
          auto ni_input = binder["input_0"];
          auto ni_output = binder["com_microsoft_DequantizeLinear_6"];

          std::vector<std::string> ns = vaip::dd::get_node_names(graph, binder);
          MY_LOG(1) << "found match at " << ns.front();

          auto input_scale = node_arg_get_const_data_as_float(
              *graph, *(binder["constant_0"]).node_arg);
          auto input_zp = node_arg_get_const_data_as_u16(
              *graph, *(binder["constant_1"]).node_arg);
          auto output_scale = node_arg_get_const_data_as_float(
              *graph, *(binder["constant_9"]).node_arg);
          auto output_zp = node_arg_get_const_data_as_u16(
              *graph, *(binder["constant_10"]).node_arg);

          std::vector<int32_t> qdq_coeffs(16, 0);

          // 0,1,2, - input_dequant -enable
          // 3,4,5, - output_quant -disable
          qdq_coeffs[0] = input_zp;
          qdq_coeffs[1] = vaip::dd::qmatmulcalc::float_to_bfloat16(input_scale);
          qdq_coeffs[2] = 1; // in_dequant_enable
          qdq_coeffs[3] = output_zp;
          qdq_coeffs[4] =
              vaip::dd::qmatmulcalc::float_to_bfloat16(1.0f / output_scale);
          qdq_coeffs[5] = 0; // out_quant_enable
          auto node_name = node_arg_get_name(*ni_output.node_arg);
          std::string qdq_name = std::string(node_name + "_qdq_");
          auto& qdq_arg = vaip::dd::insert_named_tensor_in_graph<int32_t>(
              graph, qdq_name, qdq_coeffs,
              std::vector({(int64_t)qdq_coeffs.size()}));

          std::vector<std::string> in_dtypes = {
              vaip::dd::nodearg_dtype_to_string(*ni_input.node_arg), "int32"};
          std::vector<std::string> out_dtypes = {
              "bfloat16"}; //{vaip::dd::nodearg_dtype_to_string(*ni_output.node_arg)};
          NodeBuilder(*graph, self_)
              .set_op_type("QL2norm", "com.xilinx")
              .set_input_node_args({ni_input.node_arg, &qdq_arg})
              .clone_attrs(*ni_output.node)
              .clone_data_type(*ni_output.node_arg)
              .clone_shape(*ni_output.node_arg)
              .set_anchor_point1(*ni_output.node_arg)
              .add("in_dtypes", in_dtypes)
              .add("out_dtypes", out_dtypes)
              .build();

          return true;
        });
  }
  void process(IPass& self, Graph& graph) { create_rule(&self)->apply(&graph); }

public:
  IPass& self_;
};
} // namespace

DEFINE_VAIP_PASS(MergeQL2norm, vaip_pass_dd_merge_ql2norm)