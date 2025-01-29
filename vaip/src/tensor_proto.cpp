/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#include "./xir_ops/xir_ops_defs.hpp"
//
#include <exception>
#include <glog/logging.h>
// include glog/logging.h to define CHECK before include vaip_plugin.hpp

#include "./config.hpp"
#include "./pass_imp.hpp"

#include "vaip/util.hpp"

#include "vaip/tensor_proto.hpp"
#include "vaip/vaip_ort.hpp"
#include "vaip/vaip_plugin.hpp"
#include "version_info.hpp"
#include "vitis/ai/env_config.hpp"
#include <vaip/custom_op.h>
#include <vaip/my_ort.h>
#include <vaip/vaip_ort_api.h>

// #include "core/common/status.h"
#include <memory>
#include <xir/graph/graph.hpp>

namespace vaip_core {

VAIP_DLL_SPEC TensorProtoPtr tensor_proto_new_floats(
    const std::string& name, const std::vector<int64_t>& shape,
    const std::vector<float>& data) {
  return TensorProtoPtr(
      VAIP_ORT_API(tensor_proto_new_floats)(name, shape, data));
}
#if VAIP_ORT_API_MAJOR >= 3

VAIP_DLL_SPEC TensorProtoPtr tensor_proto_new_doubles(
    const std::string& name, const std::vector<int64_t>& shape,
    const std::vector<double>& data) {
  return TensorProtoPtr(
      VAIP_ORT_API(tensor_proto_new_doubles)(name, shape, data));
}

VAIP_DLL_SPEC TensorProtoPtr tensor_proto_new_bf16(
    const std::string& name, const std::vector<int64_t>& shape,
    const std::vector<int16_t>& data) {
  return TensorProtoPtr(VAIP_ORT_API(tensor_proto_new_bf16)(name, shape, data));
}

VAIP_DLL_SPEC TensorProtoPtr tensor_proto_new_fp16(
    const std::string& name, const std::vector<int64_t>& shape,
    const std::vector<int16_t>& data) {
  return TensorProtoPtr(VAIP_ORT_API(tensor_proto_new_fp16)(name, shape, data));
}
#endif

VAIP_DLL_SPEC TensorProtoPtr
tensor_proto_new_i32(const std::string& name, const std::vector<int64_t>& shape,
                     const std::vector<int32_t>& data) {
  return TensorProtoPtr(VAIP_ORT_API(tensor_proto_new_i32)(name, shape, data));
}
VAIP_DLL_SPEC TensorProtoPtr
tensor_proto_new_i64(const std::string& name, const std::vector<int64_t>& shape,
                     const std::vector<int64_t>& data) {
  return TensorProtoPtr(VAIP_ORT_API(tensor_proto_new_i64)(name, shape, data));
}
VAIP_DLL_SPEC TensorProtoPtr
tensor_proto_new_i8(const std::string& name, const std::vector<int64_t>& shape,
                    const std::vector<int8_t>& data) {
  return TensorProtoPtr(VAIP_ORT_API(tensor_proto_new_i8)(name, shape, data));
}

#if VAIP_ORT_API_MAJOR >= 3
VAIP_DLL_SPEC TensorProtoPtr
tensor_proto_new_i16(const std::string& name, const std::vector<int64_t>& shape,
                     const std::vector<int16_t>& data) {
  return TensorProtoPtr(VAIP_ORT_API(tensor_proto_new_i16)(name, shape, data));
}

VAIP_DLL_SPEC TensorProtoPtr
tensor_proto_new_u8(const std::string& name, const std::vector<int64_t>& shape,
                    const std::vector<uint8_t>& data) {
  return TensorProtoPtr(VAIP_ORT_API(tensor_proto_new_u8)(name, shape, data));
}

VAIP_DLL_SPEC TensorProtoPtr
tensor_proto_new_u16(const std::string& name, const std::vector<int64_t>& shape,
                     const std::vector<uint16_t>& data) {
  return TensorProtoPtr(VAIP_ORT_API(tensor_proto_new_u16)(name, shape, data));
}

VAIP_DLL_SPEC TensorProtoPtr
tensor_proto_new_u32(const std::string& name, const std::vector<int64_t>& shape,
                     const std::vector<uint32_t>& data) {
  return TensorProtoPtr(VAIP_ORT_API(tensor_proto_new_u32)(name, shape, data));
}

VAIP_DLL_SPEC TensorProtoPtr
tensor_proto_new_u64(const std::string& name, const std::vector<int64_t>& shape,
                     const std::vector<uint64_t>& data) {
  return TensorProtoPtr(VAIP_ORT_API(tensor_proto_new_u64)(name, shape, data));
}

#endif

#if VAIP_ORT_API_MAJOR >= 13
VAIP_DLL_SPEC TensorProtoPtr
tensor_proto_new_i4(const std::string& name, const std::vector<int64_t>& shape,
                    const std::vector<int8_t>& data) {
  return TensorProtoPtr(VAIP_ORT_API(tensor_proto_new_i4)(name, shape, data));
}

VAIP_DLL_SPEC TensorProtoPtr
tensor_proto_new_u4(const std::string& name, const std::vector<int64_t>& shape,
                    const std::vector<uint8_t>& data) {
  return TensorProtoPtr(VAIP_ORT_API(tensor_proto_new_u4)(name, shape, data));
}

#endif

VAIP_DLL_SPEC gsl::span<const char>
tensor_proto_as_raw(const onnxruntime::Graph& graph,
                    const TensorProto& tensor_proto) {
#if VAIP_ORT_API_MAJOR >= 9
  auto raw_data = VAIP_ORT_API(tensor_proto_as_raw)(graph, tensor_proto);
#else
  auto raw_data = VAIP_ORT_API(tensor_proto_as_raw)(tensor_proto);
#endif
  return raw_data;
}

template <typename T>
static gsl::span<const T> tensor_proto_as(const onnxruntime::Graph& graph,
                                          const TensorProto& tensor_proto,
                                          int data_type) {

  auto tensor_data_type = VAIP_ORT_API(tensor_proto_data_type)(tensor_proto);
  CHECK_EQ(tensor_data_type, data_type);
  auto raw_data = tensor_proto_as_raw(graph, tensor_proto);
  auto p = reinterpret_cast<const T*>(raw_data.data());
  auto num_of_element = raw_data.size() / sizeof(T);
  return gsl::span<const T>(p, p + num_of_element);
}

VAIP_DLL_SPEC float tensor_proto_as_float(const onnxruntime::Graph& graph,
                                          const TensorProto& tensor) {
  auto v = tensor_proto_as_floats(graph, tensor);
  auto shape = tensor_proto_get_shape(tensor);
  // CHECK(shape.empty()) << "tensor proto is not float";
  CHECK_EQ(v.size(), 1u);
  return v[0];
}

VAIP_DLL_SPEC double tensor_proto_as_double(const onnxruntime::Graph& graph,
                                            const TensorProto& tensor) {
  auto v = tensor_proto_as_doubles(graph, tensor);
  auto shape = tensor_proto_get_shape(tensor);
  CHECK(shape.empty()) << "tensor proto is not double";
  CHECK_EQ(v.size(), 1u);
  return v[0];
}

VAIP_DLL_SPEC int16_t tensor_proto_as_bf16(const onnxruntime::Graph& graph,
                                           const TensorProto& tensor) {
  auto v = tensor_proto_as_bf16s(graph, tensor);
  auto shape = tensor_proto_get_shape(tensor);
  CHECK(shape.empty()) << "tensor proto is not bf16";
  CHECK_EQ(v.size(), 1u);
  return v[0];
}

VAIP_DLL_SPEC int16_t tensor_proto_as_fp16(const onnxruntime::Graph& graph,
                                           const TensorProto& tensor) {
  auto v = tensor_proto_as_fp16s(graph, tensor);
  auto shape = tensor_proto_get_shape(tensor);
  CHECK(shape.empty()) << "tensor proto is not fp16";
  CHECK_EQ(v.size(), 1u);
  return v[0];
}

VAIP_DLL_SPEC int8_t tensor_proto_as_i8(const onnxruntime::Graph& graph,
                                        const TensorProto& tensor) {
  auto v = tensor_proto_as_i8s(graph, tensor);
  auto shape = tensor_proto_get_shape(tensor);
  CHECK(shape.empty()) << "tensor proto is not i8s";
  CHECK_EQ(v.size(), 1u);
  return v[0];
}
VAIP_DLL_SPEC uint8_t tensor_proto_as_u8(const onnxruntime::Graph& graph,
                                         const TensorProto& tensor) {
  auto v = tensor_proto_as_u8s(graph, tensor);
  auto shape = tensor_proto_get_shape(tensor);
  CHECK(shape.empty()) << "tensor proto is not u8s";
  CHECK_EQ(v.size(), 1u);
  return v[0];
}

VAIP_DLL_SPEC int16_t tensor_proto_as_i16(const onnxruntime::Graph& graph,
                                          const TensorProto& tensor) {
  auto v = tensor_proto_as_i16s(graph, tensor);
  auto shape = tensor_proto_get_shape(tensor);
  CHECK(shape.empty()) << "tensor proto is not i16s";
  CHECK_EQ(v.size(), 1u);
  return v[0];
}

VAIP_DLL_SPEC uint16_t tensor_proto_as_u16(const onnxruntime::Graph& graph,
                                           const TensorProto& tensor) {
  auto v = tensor_proto_as_u16s(graph, tensor);
  auto shape = tensor_proto_get_shape(tensor);
  CHECK(shape.empty()) << "tensor proto is not u16s";
  CHECK_EQ(v.size(), 1u);
  return v[0];
}

VAIP_DLL_SPEC int32_t tensor_proto_as_i32(const onnxruntime::Graph& graph,
                                          const TensorProto& tensor) {
  auto v = tensor_proto_as_i32s(graph, tensor);
  auto shape = tensor_proto_get_shape(tensor);
  CHECK(shape.empty()) << "tensor proto is not i32s";
  CHECK_EQ(v.size(), 1u);
  return v[0];
}

VAIP_DLL_SPEC uint32_t tensor_proto_as_u32(const onnxruntime::Graph& graph,
                                           const TensorProto& tensor) {
  auto v = tensor_proto_as_u32s(graph, tensor);
  auto shape = tensor_proto_get_shape(tensor);
  CHECK(shape.empty()) << "tensor proto is not u32s";
  CHECK_EQ(v.size(), 1u);
  return v[0];
}

VAIP_DLL_SPEC int64_t tensor_proto_as_i64(const onnxruntime::Graph& graph,
                                          const TensorProto& tensor) {
  auto v = tensor_proto_as_i64s(graph, tensor);
  auto shape = tensor_proto_get_shape(tensor);
  CHECK(shape.empty()) << "tensor proto is not i64";
  CHECK_EQ(v.size(), 1u);
  return v[0];
}

VAIP_DLL_SPEC uint64_t tensor_proto_as_u64(const onnxruntime::Graph& graph,
                                           const TensorProto& tensor) {
  auto v = tensor_proto_as_u64s(graph, tensor);
  auto shape = tensor_proto_get_shape(tensor);
  CHECK(shape.empty()) << "tensor proto is not u64";
  CHECK_EQ(v.size(), 1u);
  return v[0];
}

void TensorProtoDeleter::operator()(TensorProto* p) const {
  VAIP_ORT_API(tensor_proto_delete)(p);
}

VAIP_DLL_SPEC
int8_t get_int4_value(gsl::span<const int8_t> data, size_t idx) {
  constexpr int8_t lookup_table_[16] = {0,  1,  2,  3,  4,  5,  6,  7,
                                        -8, -7, -6, -5, -4, -3, -2, -1};
  // Process a full byte at once
  uint8_t byte = static_cast<uint8_t>(data[idx >> 1]);
  // Use bit manipulation instead of branching
  uint8_t value = (byte >> ((idx & 1) << 2)) & 0xf;
  // Use lookup table instead of conditional
  return lookup_table_[value];
}

VAIP_DLL_SPEC
uint8_t get_uint4_value(gsl::span<const uint8_t> data, size_t idx) {
  size_t byte_idx = idx / 2;
  uint8_t value = data[byte_idx];
  if (idx & 1) { // odd, upper
    value = value >> 4;
  } else {
    value = value & 0xf;
  }
  return value;
}

VAIP_DLL_SPEC
gsl::span<const int8_t> tensor_proto_as_i4s(const onnxruntime::Graph& graph,
                                            const TensorProto& tensor) {
  return tensor_proto_as<int8_t>(graph, tensor,

                                 onnx::TensorProto_DataType_INT4);
}
VAIP_DLL_SPEC
gsl::span<const uint8_t> tensor_proto_as_u4s(const onnxruntime::Graph& graph,
                                             const TensorProto& tensor) {
  return tensor_proto_as<uint8_t>(graph, tensor,
                                  onnx::TensorProto_DataType_UINT4);
}

VAIP_DLL_SPEC
gsl::span<const int8_t> tensor_proto_as_i8s(const onnxruntime::Graph& graph,
                                            const TensorProto& tensor) {
  return tensor_proto_as<int8_t>(graph, tensor,
                                 onnx::TensorProto_DataType_INT8);
}
VAIP_DLL_SPEC
gsl::span<const uint8_t> tensor_proto_as_u8s(const onnxruntime::Graph& graph,
                                             const TensorProto& tensor) {
  return tensor_proto_as<uint8_t>(graph, tensor,
                                  onnx::TensorProto_DataType_UINT8);
}
VAIP_DLL_SPEC
gsl::span<const uint16_t> tensor_proto_as_u16s(const onnxruntime::Graph& graph,
                                               const TensorProto& tensor) {
  return tensor_proto_as<uint16_t>(graph, tensor,
                                   onnx::TensorProto_DataType_UINT16);
}
VAIP_DLL_SPEC
gsl::span<const int16_t> tensor_proto_as_i16s(const onnxruntime::Graph& graph,
                                              const TensorProto& tensor) {
  return tensor_proto_as<int16_t>(graph, tensor,
                                  onnx::TensorProto_DataType_INT16);
}

VAIP_DLL_SPEC
gsl::span<const int32_t> tensor_proto_as_i32s(const onnxruntime::Graph& graph,
                                              const TensorProto& tensor) {
  return tensor_proto_as<int32_t>(graph, tensor,
                                  onnx::TensorProto_DataType_INT32);
}

VAIP_DLL_SPEC
gsl::span<const uint32_t> tensor_proto_as_u32s(const onnxruntime::Graph& graph,
                                               const TensorProto& tensor) {
  return tensor_proto_as<uint32_t>(graph, tensor,
                                   onnx::TensorProto_DataType_UINT32);
}

VAIP_DLL_SPEC
gsl::span<const int64_t> tensor_proto_as_i64s(const onnxruntime::Graph& graph,
                                              const TensorProto& tensor) {
  return tensor_proto_as<int64_t>(graph, tensor,
                                  onnx::TensorProto_DataType_INT64);
}
VAIP_DLL_SPEC
gsl::span<const uint64_t> tensor_proto_as_u64s(const onnxruntime::Graph& graph,
                                               const TensorProto& tensor) {
  return tensor_proto_as<uint64_t>(graph, tensor,
                                   onnx::TensorProto_DataType_UINT64);
}

VAIP_DLL_SPEC
gsl::span<const float> tensor_proto_as_floats(const onnxruntime::Graph& graph,
                                              const TensorProto& tensor) {
  return tensor_proto_as<float>(graph, tensor,
                                onnx::TensorProto_DataType_FLOAT);
}

VAIP_DLL_SPEC
gsl::span<const double> tensor_proto_as_doubles(const onnxruntime::Graph& graph,
                                                const TensorProto& tensor) {
  return tensor_proto_as<double>(graph, tensor,
                                 onnx::TensorProto_DataType_DOUBLE);
}

VAIP_DLL_SPEC
gsl::span<const int16_t> tensor_proto_as_bf16s(const onnxruntime::Graph& graph,
                                               const TensorProto& tensor) {
  return tensor_proto_as<int16_t>(graph, tensor,
                                  onnx::TensorProto_DataType_BFLOAT16);
}

VAIP_DLL_SPEC
gsl::span<const int16_t> tensor_proto_as_fp16s(const onnxruntime::Graph& graph,
                                               const TensorProto& tensor) {
  return tensor_proto_as<int16_t>(graph, tensor,
                                  onnx::TensorProto_DataType_FLOAT16);
}
} // namespace vaip_core
