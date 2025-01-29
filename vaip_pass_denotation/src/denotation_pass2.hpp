/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#pragma once
#include <string>
#include <vector>

#include "vaip/vaip.hpp"
namespace vaip_pass_denotation {
using namespace vaip_core;
using denotation_t = std::string;
using layout_t = std::vector<denotation_t>;
class NodeActionState {
public:
  struct denotation_view_t {
    denotation_t& denotation;
  };
  NodeActionState(const Node& node);
  void negotiate_layout(std::vector<denotation_view_t> views);
  denotation_t& get_denotation(size_t input_or_output, size_t arg_idx,
                               size_t dim_idx);
  layout_t& get_layout(size_t input_or_output, size_t arg_idx);

  void check_is_modified();
  bool is_modified() const { return error_counter_ == 0 && modified_; }
  void update_layout();
  void set_input_layout(size_t arg_idx, const layout_t& layout);
  void set_output_layout(const layout_t& layout1);
  void copy_input_layout(size_t input_index);
  void copy_all_input();
  void set_broadcast_layout();
  void handle_reshape();
  void handle_transpose();
  void handle_gemm(const Node& node);
  void handle_squeeze(const Graph& graph, const Node& squeeze);

public:
  int modified_ = 0;
  int error_counter_ = 0;
  const Node* node_ = nullptr;
  std::vector<const NodeArg*> inputs_ = {};
  std::vector<std::unique_ptr<layout_t>> input_layouts_ = {};
  std::vector<const NodeArg*> outputs_ = {};
  std::vector<std::unique_ptr<layout_t>> output_layouts_ = {};
  friend class NodeDenotation;
};

class NodeDenotation {
public:
  static constexpr auto null_node_action = [](NodeActionState& self,
                                              Graph& graph, const Node& node) {
    return false;
  };
  using node_action_t =
      std::function<void(NodeActionState& self, Graph&, const Node&)>;

public:
  explicit NodeDenotation() = default;
  bool apply(Graph& graph, const Node& node);

  NodeDenotation& action(const node_action_t& action);
  NodeDenotation& set_output_layout(const layout_t& layout);
  NodeDenotation& set_input_layout(size_t idx, const layout_t& layout);
  NodeDenotation& copy_input_layout(size_t input_index);
  NodeDenotation& copy_all_input();
  NodeDenotation& set_broadcast_layout();

private:
private:
  std::vector<node_action_t> actions_ = {};
};

bool denotation_node(Graph& graph, const Node& node);

struct DenotationPass {
  DenotationPass(IPass& self) {}
  ~DenotationPass();
  bool process(IPass& self, Graph& graph, const Node& node) {
    return vaip_pass_denotation::denotation_node(graph, node);
  }
  const Graph* graph_ = nullptr;
};
bool layout_all_set(const layout_t& layout);
bool layout_none_set(const layout_t& layout);
bool layout_partial_set(const layout_t& layout);

} // namespace vaip_pass_denotation
