/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
#pragma once
#include "pass.hpp"
namespace vaip_core {
class WithCurrentGraph {
public:
  WithCurrentGraph(Graph* graph, IPass* pass) {
    graph_ = graph;
    pass_ = pass;
  }
  ~WithCurrentGraph() {
    pass_->add_context_resource(
        "__current_graph", std::shared_ptr<void>((void*)graph_, [](void*) {}));
  }

private:
  Graph* graph_;
  IPass* pass_;
};
} // namespace vaip_core
