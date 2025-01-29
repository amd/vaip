/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#pragma once
#include <regex>

#include "vaip/pattern.hpp"
namespace vaip_core {

class PatternNode : public Pattern {
public:
  explicit PatternNode(int id, const std::string& op_type,
                       std::vector<std::shared_ptr<Pattern>> args,
                       std::vector<bool> is_args_optional);
  ~PatternNode();

public:
  virtual BinderBuilderPtr
  match_uncached(const onnxruntime::Graph& graph, const NodeInput& node_input,
                 const BinderBuilder& cached_binder) const override final;
  virtual std::string debug_string() const override;

  virtual void dump_to_proto_imp(RootPatternProto& pattern_proto,
                                 PatternProto& this_proto) const override final;

private:
  const std::string op_type_;
  std::vector<std::shared_ptr<Pattern>> args_;
  std::vector<bool> is_args_optional_;
};
} // namespace vaip_core
