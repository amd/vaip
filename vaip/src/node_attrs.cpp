/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#include <glog/logging.h>

#include "vaip/node_attr.hpp"
#include <vaip/vaip_ort_api.h>

namespace vaip_core {
std::string attr_proto_as_string(const AttributeProto& attr) { return "TODO"; }
std::vector<int64_t> tensor_proto_get_shape(const TensorProto& tensor_proto) {
  auto shape = VAIP_ORT_API(tensor_proto_get_shape_unsafe)(tensor_proto);
  CHECK(shape.get() != nullptr)
      << "tensor_proto_get_shape_unsafe should not return null shape";
  return *shape;
}

std::string data_type_to_string(int elem_type) {
  auto ret = std::string();
  if (elem_type ==
      ONNX_NAMESPACE::TensorProto_DataType::TensorProto_DataType_INT8) {
    ret = "int8";
  } else if (elem_type ==
             ONNX_NAMESPACE::TensorProto_DataType::TensorProto_DataType_UINT8) {
    ret = "uint8";
  } else if (elem_type ==
             ONNX_NAMESPACE::TensorProto_DataType::TensorProto_DataType_INT32) {
    ret = "int32";
  } else if (elem_type ==
             ONNX_NAMESPACE::TensorProto_DataType::TensorProto_DataType_INT64) {
    ret = "int64";
  } else if (elem_type ==
             ONNX_NAMESPACE::TensorProto_DataType::TensorProto_DataType_FLOAT) {
    ret = "float32";
  } else if (elem_type == ONNX_NAMESPACE::TensorProto_DataType::
                              TensorProto_DataType_FLOAT16) {
    // It seems that FP16 is float16, and BFLOAT16 is bf16. but xir don't
    // support it or don't care now. test case: adobe fp16 model.
    ret = "float16";
  } else if (elem_type ==
             ONNX_NAMESPACE::TensorProto_DataType::TensorProto_DataType_BOOL) {
    // xir does not support bool.
    // test case: /home/public/bevdet/LS_int.onnx
    ret = "int1";
  } else if (elem_type == ONNX_NAMESPACE::TensorProto_DataType::
                              TensorProto_DataType_UINT16) {
    ret = "uint16";
  } else if (elem_type ==
             ONNX_NAMESPACE::TensorProto_DataType::TensorProto_DataType_INT16) {
    ret = "int16";
  } else if (elem_type == ONNX_NAMESPACE::TensorProto_DataType::
                              TensorProto_DataType_BFLOAT16) {
    ret = "bfloat16";
  } else if (elem_type ==
             ONNX_NAMESPACE::TensorProto_DataType::TensorProto_DataType_INT4) {
    ret = "int4";
  } else if (elem_type ==
             ONNX_NAMESPACE::TensorProto_DataType::TensorProto_DataType_UINT4) {
    ret = "uint4";
  } else {
    LOG(FATAL) << "unknown type:" << elem_type;
  }
  return ret;
}

NodeAttr::NodeAttr(const std::string& name, int64_t value)
    : attribute_proto_{
          AttributeProtoPtr(VAIP_ORT_API(attr_proto_new_int)(name, value))} {}

NodeAttr::NodeAttr(const std::string& name, float value)
    : attribute_proto_{VAIP_ORT_API(attr_proto_new_float)(name, value)} {}

NodeAttr::NodeAttr(const std::string& name, const std::string& value)
    : attribute_proto_{VAIP_ORT_API(attr_proto_new_string)(name, value)} {}

NodeAttr::NodeAttr(const std::string& name, const TensorProto& value)
    : attribute_proto_{VAIP_ORT_API(attr_proto_new_tensor)(name, value)} {}

NodeAttr::NodeAttr(const std::string& name, const std::vector<int64_t>& value)
    : attribute_proto_{VAIP_ORT_API(attr_proto_new_ints)(name, value)} {}

NodeAttr::NodeAttr(const std::string& name, const std::vector<float>& value)
    : attribute_proto_{VAIP_ORT_API(attr_proto_new_floats)(name, value)} {}

NodeAttr::NodeAttr(const std::string& name,
                   const std::vector<std::string>& value)
    : attribute_proto_{VAIP_ORT_API(attr_proto_new_strings)(name, value)} {}

NodeAttr::NodeAttr(const std::string& name, AttributeProtoPtr ptr)
    : attribute_proto_{std::move(ptr)} {
  VAIP_ORT_API(attr_proto_set_name)(attribute_proto_.get(), name);
}

AttributeProto& NodeAttr::get() { return *attribute_proto_; }
const AttributeProto& NodeAttr::get() const { return *attribute_proto_; }

NodeAttributesBuilder::NodeAttributesBuilder(size_t capacity) : attrs_{} {
  attrs_.reserve(capacity);
}

NodeAttributesPtr NodeAttributesBuilder::build() {
  auto ret = NodeAttributesPtr(VAIP_ORT_API(node_attributes_new)());
  for (auto& node_attr : attrs_) {
    AttributeProto& attr_proto = node_attr.get();
    VAIP_ORT_API(node_attributes_add)(*ret, std::move(attr_proto));
  }
  attrs_.clear();
  return ret;
}

void NodeAttributesBuilder::merge_into(Node& node) {
  merge_into(VAIP_ORT_API(node_get_attributes)(node));
}

void NodeAttributesBuilder::merge_into(NodeAttributes& attrs) {
  for (auto& attr : attrs_) {
    AttributeProto& attr_proto = attr.get();
    VAIP_ORT_API(node_attributes_add)(attrs, std::move(attr_proto));
  }
}

} // namespace vaip_core
