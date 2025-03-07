/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
#include <assert.h>
// #include <onnxruntime_cxx_api.h>
// #include "initialize_vaip.hpp"

#include "vitis/ai/env_config.hpp"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

#include "fuse.hpp"

DEF_ENV_PARAM_2(DD_SUPPORTED_OPS_JSON, "", std::string)

namespace fs = std::filesystem;

using namespace std;
using namespace vaip_core;

namespace dd {
void writeAdjacencyListToFile(dd::Graph& adjacency_list,
                              const std::string& filename) {
  std::ofstream outfile(filename);
  if (!outfile.is_open()) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return;
  }

  for (const auto& entry : adjacency_list) {
    outfile << std::to_string(entry.first) << ": [";
    for (size_t i = 0; i < entry.second.size(); ++i) {
      outfile << std::to_string(entry.second[i]);
      if (i < entry.second.size() - 1) {
        outfile << ", ";
      }
    }
    outfile << "]" << std::endl;
  }

  outfile.close();
}

void writeResToFile(std::map<int, std::string> res) {

  std::ofstream outFile("node_idx.txt");
  if (!outFile) {
    std::cerr << "Error opening file for writing!" << std::endl;
  }

  for (const auto& pair : res) {
    outFile << pair.first << ": " << pair.second << std::endl;
  }

  outFile.close();
}

std::tuple<dd::label_map, dd::Graph, std::map<int, std::string>,
           std::map<dd::node_ind_t, std::string>>
partition_onnx_model(onnxruntime::Graph& graph) {
  std::vector<std::string> supported_ops = {"QMatMul",
                                            "QMatMulAddGelu",
                                            "QGemmvGelu",
                                            "QMatMulAdd",
                                            "QLayerNorm",
                                            "QEltWiseAdd",
                                            "QMHAGRPB",
                                            "QConcateOPs",
                                            "QConv",
                                            "QConcat",
                                            "QGroupNorm",
                                            "QSlice",
                                            "QELWEMUL_qdq",
                                            "QELWEMUL_mxgan",
                                            "IConv",
                                            "QReshapeTranspose",
                                            "QGlobalAvgPool",
                                            "QMHACHANNEL",
                                            "QMHAWINDOW",
                                            "QMHA",
                                            "DQAdd",
                                            "mzdk5MHA",
                                            "QSilu",
                                            "SILU",
                                            "QGelu",
                                            "QMatMulDynamic",
                                            "FlatRMSAdd",
                                            "QMatMulDynamicSoftmax",
                                            "QMulSoftmax",
                                            "xcom-conv2d",
                                            "QBroadcastAdd",
                                            "Mladfsoftmax",
                                            "MLADFADD",
                                            "QuantOP",
                                            "QResize",
                                            "DeQuantOP",
                                            "QConv2MatMul",
                                            "QSigmoid",
                                            "MLADFRMSNORM",
                                            "AttentionMaskPrePro",
                                            "AttentionMaskPrePro_win25",
                                            "Mladfelwmul",
                                            "ELWMUL",
                                            "MLADFMATMULA16A16",
                                            "DPS",
                                            "MladfMatMul",
                                            "FlatMLP",
                                            "Qtanh_lpnorm",
                                            "QL2norm",
                                            "L2_Norm",
                                            "QReduceSum",
                                            "QActConstAdd",
                                            "QEltWiseDiv",
                                            "QExpand",
                                            "Qbias_add",
                                            "QDeMHA",
                                            "QGatherDivAdd",
                                            "QIntEltwiseAdd",
                                            "QIntEltwiseMul",
                                            "QConv2MatMulSilu",
                                            "QBatchMatMul"};
  std::map<std::string, std::string> excluded_op_names = {};

  std::string filename = ENV_PARAM(DD_SUPPORTED_OPS_JSON);
  // LOG(DEBUG) << "DD supported ops json : " << filename << std::endl;
  std::vector<std::string> new_supported_ops;
  if (filename != "") {
    std::ifstream f(filename);
    CHECK(f.is_open()) << "failed to open json file: " << filename;
    nlohmann::json data; // = nlohmann::json::parse(f);
    try {
      data = nlohmann::json::parse(f, nullptr, true);
    } catch (std::exception& e) {
      LOG(WARNING) << "Failed to parse JSON: " << filename
                   << ", Detail : " << e.what();
    }
    supported_ops = data["supported_ops"].get<std::vector<std::string>>();
    excluded_op_names = data["excluded_ops"];
  }

  // for (auto s : supported_ops)
  //   LOG(INFO) << "dd supported op : " << s << std::endl;
  // for (const auto& s : excluded_op_names)
  //   LOG(INFO) << "excluded_op  : " << s.first << s.second << std::endl;

  vaip_core::graph_resolve(graph, true);

  auto nodes = graph_get_node_in_topoligical_order(graph);

  dd::Graph adjacency_list;

  std::map<int, std::string> property;
  std::map<dd::node_ind_t, std::string> idx_node_map;
  for (auto node_idx : nodes) {
    auto node = VAIP_ORT_API(graph_get_node)(graph, node_idx);
    std::string node_domain = VAIP_ORT_API(node_op_domain)(*node);
    std::string node_name = VAIP_ORT_API(node_get_name)(*node);
    auto node_op_type = VAIP_ORT_API(node_op_type)(*node);
    idx_node_map[(int32_t)node_idx] = node_name;

    auto it = excluded_op_names.find(node_name);
    if (it != excluded_op_names.end() && it->second == node_op_type) {
      property[(int32_t)node_idx] = "CPU";
    } else if (std::find(supported_ops.begin(), supported_ops.end(),
                         node_op_type) != supported_ops.end() &&
               (node_domain == "com.amd" || node_domain == "com.xilinx")) {
      property[(int32_t)node_idx] = "AIE";
    } else {
      property[(int32_t)node_idx] = "CPU";
    }

    std::vector<dd::node_ind_t> successors;
    for (const auto& output_arg : node_get_output_node_args(*node)) {
      std::string output_name = node_arg_get_name(*output_arg);
      std::vector<const onnxruntime::Node*> consumers =
          graph_get_consumer_nodes(graph, output_name);

      for (const auto consumer_node : consumers) {
        successors.push_back(int(VAIP_ORT_API(node_get_index)(*consumer_node)));
      }

      adjacency_list[(int32_t)node_idx] = successors;
    }
  }
  // writeAdjacencyListToFile(adjacency_list, "mdsqr_adj.txt");
  // writeResToFile(idx_node_map);

  dd::label_map subgraphs =
      dd::partition_graph(adjacency_list, property, "L1", nodes);
  dd::Graph cluster = dd::subgraph_labels_to_clusters(subgraphs);
  return std::make_tuple(subgraphs, cluster, property, idx_node_map);
}
} // namespace dd