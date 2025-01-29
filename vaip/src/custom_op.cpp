/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
// clang-format off
#include <glog/logging.h>
// include glog/logging.h to define CHECK before include vaip_plugin.hpp
#include "./vaip.hpp"
#include "vaip/custom_op_imp.hpp"
// clang-format on

namespace vaip_core {
ExecutionProvider::ExecutionProvider() {}

ExecutionProvider::~ExecutionProvider() {}

CustomOp::CustomOp() {}
CustomOp::~CustomOp() {}

ExecutionProviderConcrete::ExecutionProviderConcrete(
    std::shared_ptr<const PassContext> context, const MetaDefProto& meta_def)
    : context_{context}, meta_def_{std::make_shared<MetaDefProto>(meta_def)} {}

ExecutionProviderConcrete::~ExecutionProviderConcrete() {}

template <typename T, typename = void> struct CustomOp_InitSession_t {
  static Ort::Session
  CustomOp_InitSession(const T* api, onnxruntime::Model* model,
                       const std::shared_ptr<MetaDefProto>& meta_def) {
    if (meta_def->fallback_cpu()) {
      LOG(FATAL) << "Set fallback_cpu to true. your onnxruntime does not "
                    "support model_to_proto";
    }
    return Ort::Session(nullptr);
  }
};

template <typename T>
struct CustomOp_InitSession_t<
    T, std::void_t<decltype(std::declval<T&>().model_to_proto)>> {
  static Ort::Session
  CustomOp_InitSession(const T* api, onnxruntime::Model* model,
                       const std::shared_ptr<MetaDefProto>& meta_def) {
    if (meta_def->fallback_cpu()) {
      CHECK(model);
      Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "VitisAI_VAIP_CustomOp");
      auto model_proto = api->model_to_proto(*model);
      auto mproto_string = api->model_proto_serialize_as_string(*model_proto);
      auto session =
          Ort::Session(env, mproto_string.get()->c_str(),
                       mproto_string.get()->size(), Ort::SessionOptions());
      api->model_proto_delete(model_proto);
      api->model_delete(model);
      return session;
    }
    return Ort::Session(nullptr);
  }
};
CustomOpImp::CustomOpImp(std::shared_ptr<const PassContext> context,
                         const std::shared_ptr<MetaDefProto>& meta_def,
                         onnxruntime::Model* model)
    : context_{context}, meta_def_{meta_def},
      session_{CustomOp_InitSession_t<
          vaip_core::OrtApiForVaip>::CustomOp_InitSession(vaip_core::api(),
                                                          model, meta_def)} {}
CustomOpImp::~CustomOpImp() {}
void CustomOpImp::ComputeCpu(const OrtApi* api,
                             OrtKernelContext* context) const {
  Ort::KernelContext ctx(context);
  Ort::AllocatorWithDefaultOptions allocator;
  CHECK(session_);
  std::vector<const OrtValue*> input_values;
  std::vector<OrtValue*> output_values;
  std::vector<const char*> input_names;
  std::vector<const char*> output_names;
  std::vector<Ort::AllocatedStringPtr> string_ptr;
  for (auto idx = 0u; idx < ctx.GetInputCount(); ++idx) {
    input_values.push_back(ctx.GetInput(idx));
    auto name = session_.GetInputNameAllocated(idx, allocator);
    input_names.push_back(name.get());
    string_ptr.push_back(std::move(name));
  }
  for (auto idx = 0u; idx < ctx.GetOutputCount(); ++idx) {
    auto shape =
        session_.GetOutputTypeInfo(idx).GetTensorTypeAndShapeInfo().GetShape();
    output_values.push_back(ctx.GetOutput(idx, shape));
    auto name = session_.GetOutputNameAllocated(idx, allocator);
    output_names.push_back(name.get());
    string_ptr.push_back(std::move(name));
  }
  Ort::ThrowOnError(api->Run(session_, Ort::RunOptions(), input_names.data(),
                             input_values.data(), input_names.size(),
                             output_names.data(), output_names.size(),
                             output_values.data()));
}
} // namespace vaip_core
