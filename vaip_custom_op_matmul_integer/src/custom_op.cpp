/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
#include "onnxruntime_api.hpp"

#include "./custom_op.hpp"
#include <filesystem>
#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <sstream>
#pragma once
#include "qlinear_2/qlinear_2.hpp"
#if defined(_WIN32)
#  pragma warning(disable : 4996)
#endif

namespace fs = std::filesystem;
using namespace ryzenai;

#define OUT_TYPE int32_t

namespace vaip_matmul_integer_custom_op {
MyCustomOp::MyCustomOp(std::shared_ptr<const PassContext> context,
                       const std::shared_ptr<MetaDefProto>& meta_def,
                       onnxruntime::Model* model)
    : CustomOpImp(context, meta_def, model) { //,
  impl_ = meta_def->generic_param().at("impl");
  quant_mode_ = meta_def->generic_param().at("quant_mode");

  std::string inputbin_wts = meta_def->generic_param().at("wts_file");
  auto shape_0 = stoi(meta_def->generic_param().at("wts_shape_dim_0"));
  auto shape_1 = stoi(meta_def->generic_param().at("wts_shape_dim_1"));

  wts_shape_ = std::make_tuple(shape_0, shape_1);

  unsigned int size = (unsigned int)fs::file_size(inputbin_wts);
  int8_t* wts = (int8_t*)malloc(size);

  auto infile = std::ifstream(inputbin_wts, std::ios::in | std::ios::binary);
  for (unsigned i = 0; infile.read(&((char*)wts)[i], sizeof(int8_t)); i++)
    ;

  if (impl_ == "v1") {
    if (quant_mode_ == "w8a8") {

      const std::string& a_dtype = "int8";
      const std::string& b_dtype = "int8";
      const std::string& c_dtype = "int32";
      gemm_ = std::make_shared<qlinear_2<int8_t, int8_t, int32_t>>(
          a_dtype, b_dtype, c_dtype);
      qlinear_2<int8_t, int8_t, int32_t>* ptr =
          (qlinear_2<int8_t, int8_t, int32_t>*)gemm_.get();
      ptr->initialize_weights(wts, wts_shape_);

    } else {
      const std::string& a_dtype = "int16";
      const std::string& b_dtype = "int8";
      const std::string& c_dtype = "int64";
      gemm_ = std::make_shared<qlinear_2<int16_t, int8_t, int64_t>>(
          a_dtype, b_dtype, c_dtype);
      qlinear_2<int16_t, int8_t, int64_t>* ptr =
          (qlinear_2<int16_t, int8_t, int64_t>*)gemm_.get();
      ptr->initialize_weights(wts, wts_shape_);
    }
  } else {
    throw std::runtime_error(
        "ERROR : # Implementaion is not available for this version");
  }

  wts_sum_.reserve(std::get<1>(wts_shape_));
  for (int i = 0; i < std::get<1>(wts_shape_); i++) {
    wts_sum_[i] = 0;
    for (int j = 0; j < std::get<0>(wts_shape_); j++) {
      wts_sum_[i] += wts[j * std::get<1>(wts_shape_) + i];
    }
  }
  free(wts);
}

MyCustomOp::~MyCustomOp() {}

void MyCustomOp::Compute(const OrtApi* api, OrtKernelContext* context) const {
  if (Ort::Global<void>::api_ == nullptr) {
    Ort::Global<void>::api_ = api;
  }
#ifdef PROFILE_MATMULINTEGER
  std::chrono::time_point<std::chrono::high_resolution_clock> exec_start,
      exec_stop;
  std::chrono::time_point<std::chrono::high_resolution_clock> preproc_start,
      preproc_end;
  std::chrono::time_point<std::chrono::high_resolution_clock> kernel_start,
      kernel_end;
  std::chrono::time_point<std::chrono::high_resolution_clock> scale_start,
      scale_end;
#endif
  USE_TIMER_MATMULINTEGER(exec_start =
                              std::chrono::high_resolution_clock::now());
  Ort::KernelContext ctx(context);
  auto input_tensor = ctx.GetInput(0);
  auto input_data = input_tensor.GetTensorData<uint8_t>();
  auto input_shape = input_tensor.GetTensorTypeAndShapeInfo().GetShape();
  auto input_tensor1 = ctx.GetInput(1); // Zero point
  auto input_zero_point = input_tensor1.GetTensorData<uint8_t>();
  std::vector<int64_t> out_shape;

  for (unsigned i = 0; i < (input_shape.size() - 1); i++)
    out_shape.push_back(input_shape[i]);
  out_shape.push_back(std::get<1>(wts_shape_));

  auto output_tensor = ctx.GetOutput(0, {out_shape.begin(), out_shape.end()});

  std::tuple<int, int> input_s =
      std::make_tuple((int)input_shape[input_shape.size() - 2],
                      (int)input_shape[input_shape.size() - 1]);

  auto out_base = output_tensor.GetTensorMutableData<int32_t>();

  auto _exec_start = std::chrono::high_resolution_clock::now();
  size_t in_size = std::get<0>(input_s) * std::get<1>(input_s);
  size_t out_size = std::get<0>(input_s) * std::get<1>(wts_shape_);

  if (quant_mode_ == "w8a8") {
    USE_TIMER_MATMULINTEGER(preproc_start =
                                std::chrono::high_resolution_clock::now());
    std::vector<int8_t> input_data1(in_size, 0);
    std::vector<int32_t> out_tmp(out_size, 0);

    for (size_t i = 0; i < in_size; i++) {
      input_data1[i] = (int8_t)(input_data[i] - 128);
    }
    USE_TIMER_MATMULINTEGER(preproc_end =
                                std::chrono::high_resolution_clock::now());

    USE_TIMER_MATMULINTEGER(kernel_start =
                                std::chrono::high_resolution_clock::now());

    qlinear_2<int8_t, int8_t, int32_t>* ptr =
        (qlinear_2<int8_t, int8_t, int32_t>*)gemm_.get();
    ptr->execute(input_data1.data(), input_s, out_tmp.data());

    USE_TIMER_MATMULINTEGER(kernel_end =
                                std::chrono::high_resolution_clock::now());
    USE_TIMER_MATMULINTEGER(scale_start =
                                std::chrono::high_resolution_clock::now());
    // Reference:
    // https://leimao.github.io/article/Neural-Networks-Quantization/#Quantized%20Matrix%20Multiplication:~:text=Quantized%20Matrix%20Multiplication-,Quantized%20Matrix%20Multiplication%20Mathematics,-Suppose%20we%20have
    // Assuming that the zero point of weight is zero
    for (int i = 0; i < std::get<0>(input_s); i++) {
      for (int j = 0; j < std::get<1>(wts_shape_); j++) {
        int32_t temp = out_tmp[i * std::get<1>(wts_shape_) + j];
        out_base[i * std::get<1>(wts_shape_) + j] =
            (temp - ((input_zero_point[0] - 128) * (wts_sum_[j])));
      }
    }
  } else {

    USE_TIMER_MATMULINTEGER(preproc_start =
                                std::chrono::high_resolution_clock::now());
    std::vector<int16_t> input_data1(in_size, 0);
    std::vector<int64_t> out_tmp(out_size, 0);
    for (size_t i = 0; i < in_size; i++) {
      input_data1[i] = (int16_t)(input_data[i] - 128);
    }
    USE_TIMER_MATMULINTEGER(preproc_end =
                                std::chrono::high_resolution_clock::now());

    USE_TIMER_MATMULINTEGER(kernel_start =
                                std::chrono::high_resolution_clock::now());

    qlinear_2<int16_t, int8_t, int64_t>* ptr =
        (qlinear_2<int16_t, int8_t, int64_t>*)gemm_.get();
    ptr->execute(input_data1.data(), input_s, out_tmp.data());

    USE_TIMER_MATMULINTEGER(kernel_end =
                                std::chrono::high_resolution_clock::now());
    USE_TIMER_MATMULINTEGER(scale_start =
                                std::chrono::high_resolution_clock::now());
    for (int i = 0; i < std::get<0>(input_s); i++) {
      for (int j = 0; j < std::get<1>(wts_shape_); j++) {
        int64_t temp = out_tmp[i * std::get<1>(wts_shape_) + j];
        out_base[i * std::get<1>(wts_shape_) + j] =
            (temp - ((input_zero_point[0] - 128) * (wts_sum_[j])));
      }
    }
  }
  USE_TIMER_MATMULINTEGER(scale_end =
                              std::chrono::high_resolution_clock::now());
  USE_TIMER_MATMULINTEGER(exec_stop =
                              std::chrono::high_resolution_clock::now());

#ifdef PROFILE_MATMULINTEGER
  std::stringstream _csv_out;
  _csv_out << "total execution, preproc, kernel_exec, scale_cal"
           << "\n";
  _csv_out << (exec_stop - exec_start) / std::chrono::microseconds(1) << ",";
  _csv_out << (preproc_end - preproc_start) / std::chrono::microseconds(1)
           << ",";
  _csv_out << (kernel_end - kernel_start) / std::chrono::microseconds(1) << ",";
  _csv_out << (scale_end - scale_start) / std::chrono::microseconds(1) << ",";
  std::ofstream csv_file_out("./matmulinteger.csv",
                             std::ios::app | std::ios::out);

  csv_file_out << _csv_out.str() << std::endl;
#endif
}
} // namespace vaip_matmul_integer_custom_op
