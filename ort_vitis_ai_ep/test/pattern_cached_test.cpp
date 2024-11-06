/*
 *     The Xilinx Vitis AI Vaip in this distribution are provided under the
 * following free and permissive binary-only license, but are not provided in
 * source code form.  While the following free and permissive license is similar
 * to the BSD open source license, it is NOT the BSD open source license nor
 * other OSI-approved open source license.
 *
 *      Copyright (C) 2022 Xilinx, Inc. All rights reserved.
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

#include <glog/logging.h>

#include "./test_main.hpp"
#include "core/graph/model.h"
#include "vaip/pattern/pattern.hpp"
#include "vaip/rewrite_rule.hpp"

using namespace std;
using namespace onnxruntime;
using namespace vaip;

static inline const char* CACHED_SAMPLE_0() {
  return R"ONNX(
<
  ir_version: 7,
  opset_import: [ "" : 10 ]
>
agraph (float[N, 128] X, float[128, 10] W, float[10] B) => (float[N, 10] C)
{
    T = MatMul(X, W)
    M = Softmax(T)
    N = Relu(T)
    C = Add(M, N)
}
)ONNX";
};
static inline const char* CACHED_SAMPLE_1() {
  return R"ONNX(
<
  ir_version: 7,
  opset_import: [ "" : 10 ]
>
agraph (float[N, 128] X,  float[128, 10] W, float[10] B) => (float[N, 10] C)
{
    T = MatMul(X, W)
    M = Softmax(T)
    D = Relu(T)
    N = Relu(D)
    C = Add(M, N)
}
)ONNX";
};

using namespace std;
using namespace onnxruntime;
using namespace vaip;

class PatternCachedTest : public WithLogger {
protected:
  PatternCachedTest() : WithLogger() {}
};

class PatternCachedTestRule : public Rule {
public:
  PatternCachedTestRule();

private:
  virtual const Pattern* pattern() const override;
  virtual bool action(onnxruntime::Graph* graph,
                      binder_t& binder) const override;

private:
  std::shared_ptr<Pattern> input_;
  std::shared_ptr<Pattern> softmax_;
  std::shared_ptr<Pattern> relu_;
  std::unique_ptr<Pattern> add_;
};

PatternCachedTestRule::PatternCachedTestRule() : Rule() {
  auto builder = PatternBuilder();
  input_ = builder.wildcard();
  softmax_ = builder.node2("Softmax", {input_});
  relu_ = builder.node2("Relu", {input_});
  add_ = builder.node2("Add", {softmax_, relu_});
}

const Pattern* PatternCachedTestRule::pattern() const { return add_.get(); }

bool PatternCachedTestRule::action(onnxruntime::Graph* graph,
                                   binder_t& binder) const {
  LOG(INFO) << "=== MATCH PatternCachedTest";
  return false;
}

TEST_F(PatternCachedTest, pattern_cached_test) {
  std::shared_ptr<onnxruntime::Model> model = load_model(CACHED_SAMPLE_0());
  auto& graph = model->MainGraph();
  PatternCachedTestRule rule;
  rule.apply(&graph);
  // LOG(INFO) << "graph= " << graph.ToGraphProto().DebugString();

  LOG(INFO) << "===========";

  std::shared_ptr<onnxruntime::Model> model1 = load_model(CACHED_SAMPLE_1());
  auto& graph1 = model1->MainGraph();
  rule.apply(&graph1);
}
