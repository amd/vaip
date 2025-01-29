/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#pragma once

#include "vaip/pattern.hpp"
namespace vaip_core {

class PatternSequence : public Pattern {
public:
  explicit PatternSequence(int id,
                           gsl::span<const std::shared_ptr<Pattern>> patterns);
  ~PatternSequence();

private:
  virtual BinderBuilderPtr
  match_uncached(const onnxruntime::Graph& graph, const NodeInput& node_input,
                 const BinderBuilder& cached_binder) const override final;
  virtual std::string debug_string() const override;

private:
  std::vector<std::shared_ptr<Pattern>> patterns_;
};
} // namespace vaip_core
