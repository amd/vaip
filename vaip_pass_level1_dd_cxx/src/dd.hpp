/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
#pragma once

#include <cassert>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace dd {

using node_ind_t = int32_t;
using node_list = std::vector<node_ind_t>;
using node_set = std::set<node_ind_t>;
using Graph = std::map<node_ind_t, node_list>;
using label_map = std::map<node_ind_t, node_ind_t>;
using property_map = std::map<node_ind_t, std::string>;

class CompositeGraph {
public:
  Graph child_graph_;
  Graph parent_graph_;
  label_map labels_;
  Graph clusters_;

  std::vector<node_ind_t> input_nodes_;

  CompositeGraph(Graph adj_graph);
  void get_input_nodes();

  node_ind_t get_subgraph_label_of_node(node_ind_t node);
  node_ind_t get_subgraph_for_node(node_ind_t node);
  node_list get_next_nodes_of_node(node_ind_t node);
  node_list get_input_subgraphs();
  node_list get_nodes_in_subgraph(node_ind_t subg);
  node_list get_next_subgraphs(node_ind_t subg);
  bool is_cycle_detected(node_ind_t subg, node_set& subgs_visited,
                         node_set& subgs_in_stack);
  bool is_dag();
  void fuse(node_ind_t subgraph1, node_ind_t subgraph2);
  void fuse_all(node_list subgraphs);
  bool try_fuse(node_ind_t subgraph1, node_ind_t subgraph2);
  node_list topsort();

  void child_graph_to_parent_graph(Graph adj_graph);
};

void print_node_list(std::string str, node_list& nodes);
void print_property_map(std::string str, property_map& labels);
void print_label_map(std::string str, label_map& labels);
void print_graph(std::string str, Graph& g);

label_map partition_graph(Graph adj_graph, property_map property,
                          std::string optimization_flag,
                          const std::vector<size_t>& sorted_nodes);
Graph subgraph_labels_to_clusters(label_map subgraphs);

} // namespace dd