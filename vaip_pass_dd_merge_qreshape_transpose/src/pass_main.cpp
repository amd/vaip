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

DEF_ENV_PARAM(DEBUG_DD_MERGE_QRESHAPE_TRANSPOSE, "0")
#define MY_LOG(n)                                                              \
  LOG_IF(INFO, ENV_PARAM(DEBUG_DD_MERGE_QRESHAPE_TRANSPOSE) >= n)

namespace {
using namespace vaip_core;
struct Dd_merge_qreshape_transpose {
  Dd_merge_qreshape_transpose(IPass& self) : self_{self} {}
  std::unique_ptr<Rule> create_rule(IPass* self) {
    auto com_microsoft_QuantizeLinear_13 =
        vaip::pattern_zoo::get_pattern("m_qreshape_transpose");
    CHECK(com_microsoft_QuantizeLinear_13 != nullptr)
        << "Pattern returned is null";
    return Rule::create_rule(
        com_microsoft_QuantizeLinear_13,
        [=](onnxruntime::Graph* graph, binder_t& binder) -> bool {
          auto in_node = binder["input_0"];
          auto reshape_node_0 = binder["Reshape_5"];
          auto reshape_node_1 = binder["Reshape_12"];
          auto transpose_node = binder["Transpose_8"];
          auto out_node = binder["com_microsoft_QuantizeLinear_13"];

          std::vector<std::string> ns = vaip::dd::get_node_names(graph, binder);
          MY_LOG(1) << "found match at " << ns.front();

          // some attrs
          auto reshape_0_allowzero =
              node_get_attr_int(*reshape_node_0.node, "allowzero");
          auto reshape_1_allowzero =
              node_get_attr_int(*reshape_node_1.node, "allowzero");
          auto transpose_perm_span =
              node_get_attr_ints(*transpose_node.node, "perm");
          std::vector<int64_t> transpose_perm(transpose_perm_span.begin(),
                                              transpose_perm_span.end());
          // hard code for m3uec, may need to change
          std::vector<std::string> input_types{"uint16", "int32"};
          std::vector<std::string> output_types{"uint16"};

          std::vector<int32_t> elt_coeffs(16, 0);
          auto node_name = node_arg_get_name(*out_node.node_arg);
          std::string elt_coeff_name = std::string(node_name + "_qdq_");
          auto& elt_arg = vaip::dd::insert_named_tensor_in_graph<int32_t>(
              graph, elt_coeff_name, elt_coeffs,
              std::vector({(int64_t)elt_coeffs.size()}));
          NodeBuilder(*graph, *self)
              .set_input_node_args({in_node.node_arg, &elt_arg})
              .set_op_type("QReshapeTranspose", "com.xilinx")
              .add("nodes", ns)
              .add("reshape_0_allowzero", reshape_0_allowzero)
              .add("reshape_1_allowzero", reshape_1_allowzero)
              .add("transpose_perm", transpose_perm)
              .add("in_dtypes", input_types)
              .add("out_dtypes", output_types)
              .set_anchor_point1(*out_node.node)
              .build();
          return true;
        });
  }
  // apply the rule
  void process(IPass& self, Graph& graph) {
    MY_LOG(1) << "running merge_qreshape_transpose";
    create_rule(&self)->apply(&graph);
  }

  IPass& self_;
};
} // namespace

DEFINE_VAIP_PASS(Dd_merge_qreshape_transpose,
                 vaip_pass_dd_merge_qreshape_transpose)