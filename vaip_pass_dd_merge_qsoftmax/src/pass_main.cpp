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

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

#include "vaip/vaip.hpp"
#include "vitis/ai/env_config.hpp"

#include <utility> // for std::pair
DEF_ENV_PARAM(DEBUG_DD_MERGE_QSOFTMAX, "0")
#define MY_LOG(n) LOG_IF(INFO, ENV_PARAM(DEBUG_DD_MERGE_QSOFTMAX) >= n)

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
    { "name": "vaip_pass_dd_merge_qsoftmax",
       "plugin": "vaip-pass_dd_merge_qsoftmax",
       "disabled": false
    }
*/
namespace {
using namespace vaip_core;
struct DdMergeQsoftmax {
  DdMergeQsoftmax(IPass& self) : self_{self} {}
  std::unique_ptr<Rule> create_rule(IPass* self) {
    auto builder = PatternBuilder();
    auto a = builder.wildcard();
    auto a_s = builder.wildcard();
    auto a_z = builder.wildcard();
    auto b_s = builder.wildcard();
    auto b_z = builder.wildcard();

    auto dq1 = builder.node2("com.microsoft:DequantizeLinear", {a, a_s, a_z});
    auto softmax = builder.node2("Softmax", {dq1});
    auto q = builder.node2("com.microsoft:QuantizeLinear", {softmax, b_s, b_z});
    return Rule::create_rule(
        q, [=](onnxruntime::Graph* graph, binder_t& binder) -> bool {
          auto a_node = binder[a->get_id()];
          auto a_s_node = binder[a_s->get_id()];
          auto a_z_node = binder[a_z->get_id()];
          auto b_s_node = binder[b_s->get_id()];
          auto b_z_node = binder[b_z->get_id()];
          auto softmax_node = binder[softmax->get_id()];
          auto q_node = binder[q->get_id()];

          std::vector<std::string> in_dtypes = {"uint16", "float",  "uint16",
                                                "float",  "uint16", "uint8"};
          std::vector<std::string> out_dtypes = {
              vaip::dd::nodearg_dtype_to_string(*q_node.node_arg)};
          std::vector<uint8_t> rtp(64, 0);
          rtp[62] = 131;
          rtp[63] = 199;
          std::string rtp_initializer_name =
              node_arg_get_name(*softmax_node.node_arg) + "_rtp_";
          const std::vector<int64_t> rtp_initializer_shape = {
              (int64_t)rtp.size()};
          auto& rtp_arg = vaip::dd::insert_named_tensor_in_graph<uint8_t>(
              graph, rtp_initializer_name, rtp, rtp_initializer_shape);

          NodeBuilder(*graph, *self)
              .set_input_node_args({a_node.node_arg, a_s_node.node_arg,
                                    a_z_node.node_arg, b_s_node.node_arg,
                                    b_z_node.node_arg, &rtp_arg})
              .set_op_type("Mladfsoftmax")
              .set_anchor_point1(*q_node.node)
              .add("in_dtypes", in_dtypes)
              .add("out_dtypes", out_dtypes)
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

DEFINE_VAIP_PASS(DdMergeQsoftmax, vaip_pass_dd_merge_qsoftmax)