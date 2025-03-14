/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

/*

**/
#pragma once

#include "vaip/vaip.hpp"
#include "vart/runner_ext.hpp"
#include <algorithm>
#include <future>
#include <mutex>
#include <xrt/xrt_bo.h>

#include "../../xrt_shared_context/xrt_shared_context.hpp"

#include <chrono>
// #define PROFILE_GEMM
#ifdef PROFILE_GEMM
#  define USE_TIMER_GEMM(timer) timer
#else
#  define USE_TIMER_GEMM(timer)
#endif

namespace vaip_gemm_dynamic_custom_op {
using namespace vaip_core;
class MyCustomOp : public CustomOpImp {
public:
  MyCustomOp(std::shared_ptr<const PassContext> context,
             const std::shared_ptr<MetaDefProto>& meta_def,
             onnxruntime::Model* model);

  virtual ~MyCustomOp();

private:
  virtual void Compute(const OrtApi* api,
                       OrtKernelContext* context) const override final;
  std::shared_ptr<void> gemm_;
  std::shared_ptr<vaip::Context> context_;
  std::unique_ptr<xrt::xclbin> _xclbin;
  float a_scale_;
  float b_scale_;
  float y_scale_;
  float out_scale_;
  int requantize_out_scale_;
  mutable std::tuple<int, int> wts_shape_;
  //  std::vector<int8_t> wts_;
  //  std::vector<float> bias_;
  int float_data_index_, int8_data_index_;
  int a_input_zp_;
  int b_input_zp_;
  std::string impl_;
};

} // namespace vaip_gemm_dynamic_custom_op
