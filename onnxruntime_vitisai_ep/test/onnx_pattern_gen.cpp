/*
 *     The Xilinx Vitis AI Vaip in this distribution are provided under the
 * following free and permissive binary-only license, but are not provided in
 * source code form.  While the following free and permissive license is similar
 * to the BSD open source license, it is NOT the BSD open source license nor
 * other OSI-approved open source license.
 *
 *      Copyright (C) 2022 Xilinx, Inc. All rights reserved.
 *      Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights
 * reserved.
 *
 *      Redistribution and use in binary form only, without modification, is
 * permitted provided that the following conditions are met:
 *
 *      1. Redistributions must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 *      2. The name of Xilinx, Inc. may not be used to endorse or promote
 * products redistributed with this software without specific prior written
 * permission.
 *
 *      THIS SOFTWARE IS PROVIDED BY XILINX, INC. "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL XILINX, INC. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *      PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 */

// clang-format off
#include <glog/logging.h>

#include <limits>
#include <regex>
#include <string>
#include <fstream>
#include <unordered_map>
#include <exception>
#include "initialize_vaip.hpp"
#include <onnxruntime_cxx_api.h>
#include <vaip/vaip.hpp>
#include <vaip/vaip_ort.hpp>
#include <vaip/node.hpp>
#include <vaip/node_arg.hpp>
#include <vaip/pattern.hpp>
#include "vitis/ai/env_config.hpp"
DEF_ENV_PARAM(IGNORE_CONSTANT, "1")
DEF_ENV_PARAM_2(NODE_FORMAT, "$cxx_name<br>$shape, id=$pattern_id, ty=$type<br>$node_arg_name",std::string)
DEF_ENV_PARAM_2(INPUT_FORMAT, "$cxx_name<br>$shape, id=$pattern_id, ty=$type<br>$node_arg_name",std::string)
DEF_ENV_PARAM_2(CONSTANT_FORMAT, "$cxx_name<br>$shape, id=$pattern_id, ty=$type<br>$node_arg_name",std::string)
DEF_ENV_PARAM(ENABLE_CONSTANT_SHARING, "0")


extern "C" {
#include "./getopt.h"
}
// clang-format on

namespace {
using namespace vaip_core;

struct NodePattern {
public:
  NodePattern(const NodeInput& node_input, const std::string& hint,
              const std::shared_ptr<Pattern> pattern,
              const std::string& cxx_name,
              const std::vector<std::shared_ptr<NodePattern>>& inputs)
      : node_input_(node_input), hint_(hint), pattern_(pattern),
        cxx_name_(cxx_name), inputs_(inputs) {}
  const std::shared_ptr<Pattern> pattern() const { return pattern_; }
  const std::string& node_arg_name() const {
    return node_arg_get_name(*node_input_.node_arg);
  }
  /**
   * Builds a c++ snipets for genreate a pattern using the given hint and
   * inputs.
   *
   * @return The string,  a c++ snipets representation of the pattern.
   */
  const std::string cxx_build_self() const {
    std::ostringstream str;
    if (hint_ == "constant") {
      str << "builder.constant()";
    } else if (hint_ == "input") {
      str << "builder.wildcard()";
    } else {
      CHECK(node_input_.node != nullptr) << "hint_= " << hint_;
      auto& node = *node_input_.node;
      auto pattern_op_type = std::string();
      auto& op_type = node_op_type(node);
      auto& op_domain = node_op_domain(node);
      if (op_domain.empty()) {
        pattern_op_type = op_type;
      } else {
        pattern_op_type = op_domain + ":" + op_type;
      }
      str << "builder.node2(\"" << pattern_op_type << "\"";
      str << ",{";
      auto sep = std::string("");
      for (auto& i : inputs_) {
        str << sep << i->cxx_name_;
        sep = ",";
      }
      str << "})";
    }
    return str.str();
  }

  /**
   * Builds a string representation of the node in a mermaid diagram
   *
   * @param output the output node name, it is used for styling the end node.
   * @return The string representing a mermaid node.
   */
  const std::string
  mmd_build_self(const std::vector<std::string>& outputs) const {
    std::ostringstream str;
    auto indent = "    ";

    if (hint_ == "constant") {
      if (!ENV_PARAM(IGNORE_CONSTANT)) {
        str << indent << cxx_name_ << "{{\"" //
            << node_label(ENV_PARAM(CONSTANT_FORMAT), cxx_name_,
                          pattern_->get_id(),
                          node_input_) //
            << "\"}}" << std::endl;
      }
    } else if (hint_ == "input") {
      str << indent << cxx_name_ << "[\\\"" //
          << node_label(ENV_PARAM(INPUT_FORMAT), cxx_name_, pattern_->get_id(),
                        node_input_)
          << "\"/]" << std::endl;
    } else {
      auto node_arg_name = node_arg_get_name(*node_input_.node_arg);
      auto is_root_pattern = std::find(outputs.begin(), outputs.end(),
                                       node_arg_name) != outputs.end();
      str << indent << cxx_name_ << "[";
      if (!is_root_pattern) {
        str << "\"";
      } else {
        str << "[\"";
      }
      str << node_label(ENV_PARAM(NODE_FORMAT), cxx_name_, pattern_->get_id(),
                        node_input_); //
      if (!is_root_pattern) {
        str << "\"";
      } else {
        str << "\"]";
      }
      str << "]";
      str << std::endl;
      for (auto& ni : inputs_) {
        auto draw_it = true;
        if (ENV_PARAM(IGNORE_CONSTANT)) {
          if (ni->hint_ == "constant") {
            draw_it = false;
          }
        }
        if (draw_it) {
          str << indent << ni->cxx_name_
              << (is_root_pattern ? " --o " : " -.-> ") << cxx_name_
              << std::endl;
        }
      }
    }
    maybe_class_defs(hint_, cxx_name_, str);
    return str.str();
  }

  /**
   * Generates a node label for a mermaid diagram based on the provided format
   * and input parameters.
   *
   * @param format The format string used to generate the node label.
   * @param cxx_name The C++ name used in the node label.
   * @param pattern_id The pattern ID used in the node label.
   * @param ni The NodeInput object used in the node label.
   * @return The generated node label.
   */
  std::string node_label(const std::string format, const std::string& cxx_name,
                         int pattern_id, const NodeInput& ni) const {
    auto ret = format;
    auto str_pattern_id = std::to_string(pattern_id);
    auto str_node_arg = node_arg_as_string(*ni.node_arg);
    auto shape_ptr = node_arg_get_shape_i64(*ni.node_arg);
    auto shape = shape_ptr == nullptr
                     ? std::string("N/A")
                     : vaip_core::container_as_string(*shape_ptr);
    auto type = std::to_string(node_arg_get_element_type(*ni.node_arg));
    auto node_arg_name = node_arg_get_name(*ni.node_arg);
    auto op_type =
        ni.node == nullptr ? std::string("N/A") : node_op_type(*ni.node);
    auto op_domain =
        ni.node == nullptr ? std::string("N/A") : node_op_domain(*ni.node);
    ret = std::regex_replace(ret, std::regex("\\$pattern_id"), str_pattern_id);
    ret = std::regex_replace(ret, std::regex("\\$cxx_name"), cxx_name);
    ret =
        std::regex_replace(ret, std::regex("\\$node_arg_name"), node_arg_name);
    ret = std::regex_replace(ret, std::regex("\\$node_arg"), str_node_arg);
    ret = std::regex_replace(ret, std::regex("\\$shape"), shape);
    ret = std::regex_replace(ret, std::regex("\\$type"), type);
    ret = std::regex_replace(ret, std::regex("\\$op_type"), op_type);
    ret = std::regex_replace(ret, std::regex("\\$domain"), op_domain);
    return ret;
  }

  /**
   * @brief For generating mermaid diagram, checks if a given hint exists in the
   * class_defs map and generates class definitions accordingly.
   *
   * @param hint The hint string to check in the class_defs map.
   * @param cxx_name The name of the C++ class.
   * @param str The output stream to write the generated class definitions.
   */
  void maybe_class_defs(const std::string& hint, const std::string& cxx_name,
                        std::ostream& str) const {
    static const std::unordered_map<std::string, std::string> class_defs = {
        // Light Blue Fill with Dark Blue Stroke
        {"input", "fill:#add8e6,stroke:#00008b,stroke-width:2px;"},
        // Light Pink Fill with Dark Red Stroke
        {"Div", "fill:#ffb6c1,stroke:#8b0000,stroke-width:2px;"},
        // Light Green Fill with Dark Green Stroke (repeated for demonstration)
        {"Mul", "fill:#90ee90,stroke:#006400,stroke-width:2px;"},
        // Light Grey Fill with Charcoal Stroke (repeated for demonstration)
        {"ReLU", "fill:#d3d3d3,stroke:#36454f,stroke-width:2px;"},
        // Light Yellow Fill with Dark Orange Stroke (repeated for
        // demonstration)
        {"Concat", "fill:#ffffe0,stroke:#ff8c00,stroke-width:2px;"},
        // Light Blue Fill with Dark Blue Stroke (repeated for demonstration)
        {"MaxPool", "fill:#add8e6,stroke:#00008b,stroke-width:2px;"},
        // Light Coral Fill with Dark Red Stroke (repeated for demonstration)
        {"Conv", "fill:#f08080,stroke:#8b0000,stroke-width:2px;"},
        // Light Cyan Fill with Teal Stroke (repeated for demonstration)
        {"Gemm", "fill:#e0ffff,stroke:#008080,stroke-width:2px;"},
        // Beige Fill with Brown Stroke (repeated for demonstration)
        {"BatchNormalization", "fill:#f5f5dc,stroke:#8b4513,stroke-width:2px;"},
        // Lavender Fill with Dark Violet Stroke (repeated for demonstration)
        {"Softmax", "fill:#e6e6fa,stroke:#9400d3,stroke-width:2px;"},
        // Peach Fill with Dark Brown Stroke (repeated for demonstration)
        {"Flatten", "fill:#ffdab9,stroke:#8b4513,stroke-width:2px;"},
        // Light Pink Fill with Dark Red Stroke (repeated for demonstration)
        {"Div", "fill:#ffb6c1,stroke:#8b0000,stroke-width:2px;"},

        // Additional ONNX common operations with different style strings
        // Light Salmon Fill with Dark Red Stroke
        {"Add", "fill:#ffa07a,stroke:#8b0000,stroke-width:2px;"},
        // Light Goldenrod Yellow Fill with Dark Goldenrod Stroke
        {"Sub", "fill:#fafad2,stroke:#b8860b,stroke-width:2px;"},
        // Light Steel Blue Fill with Dark Slate Blue Stroke
        {"AveragePool", "fill:#b0c4de,stroke:#483d8b,stroke-width:2px;"},
        // Light Sea Fill with Dark Green Stroke
        {"GlobalAveragePool", "fill:#20b2aa,stroke:#006400,stroke-width:2px;"},
        // Light Sky Blue Fill with Dark Blue Stroke
        {"GlobalMaxPool", "fill:#87cefa,stroke:#00008b,stroke-width:2px;"},
        // Light Slate Gray Fill with Dark Slate Gray Stroke
        {"Dropout", "fill:#778899,stroke:#2f4f4f,stroke-width:2px;"},
        // Light Yellow Fill with Dark Orange Stroke
        {"LRN", "fill:#ffffe0,stroke:#ff8c00,stroke-width:2px;"},
        // Light Coral Fill with Dark Red Stroke
        {"PRelu", "fill:#ffb6c1,stroke:#8b0000,stroke-width:2px;"},
        {"Relu", "fill:#ffb6c1,stroke:#8b0000,stroke-width:2px;"},
        // Light Cyan Fill with Teal Stroke
        {"LeakyRelu", "fill:#e0ffff,stroke:#008080,stroke-width:2px;"},
        // Light Pink Fill with Dark Red Stroke
        {"Sigmoid", "fill:#ffb6c1,stroke:#8b0000,stroke-width:2px;"},
        // Light Green Fill with Dark Green Stroke
        {"Tanh", "fill:#90ee90,stroke:#006400,stroke-width:2px;"},
        // Light Grey Fill with Charcoal Stroke
        {"HardSigmoid", "fill:#d3d3d3,stroke:#36454f,stroke-width:2px;"},
        // Light Yellow Fill with Dark Orange Stroke
        {"HardSwish", "fill:#ffffe0,stroke:#ff8c00,stroke-width:2px;"},
        // Light Blue Fill with Dark Blue Stroke
        {"Elu", "fill:#add8e6,stroke:#00008b,stroke-width:2px;"},
        // Light Coral Fill with Dark Red Stroke
        {"Selu", "fill:#f08080,stroke:#8b0000,stroke-width:2px;"},
        // Light Cyan Fill with Teal Stroke
        {"ThresholdedRelu", "fill:#e0ffff,stroke:#008080,stroke-width:2px;"},
        // Beige Fill with Brown Stroke
        {"Softplus", "fill:#f5f5dc,stroke:#8b4513,stroke-width:2px;"},
        // Lavender Fill with Dark Violet Stroke
        {"Softsign", "fill:#e6e6fa,stroke:#9400d3,stroke-width:2px;"},
        // Peach Fill with Dark Brown Stroke
        {"LogSoftmax", "fill:#ffdab9,stroke:#8b4513,stroke-width:2px;"},
        // Light Salmon Fill with Dark Red Stroke
        {"QuantizeLinear", "fill:#ffa07a,stroke:#8b0000,stroke-width:2px;"},
        {"com_microsoft_QuantizeLinear",
         "fill:#ffa07a,stroke:#8b0000,stroke-width:2px;"},
        // Light Goldenrod Yellow Fill with Dark Goldenrod Stroke
        {"DequantizeLinear", "fill:#fafad2,stroke:#b8860b,stroke-width:2px;"},
        {"com_microsoft_DequantizeLinear",
         "fill:#fafad2,stroke:#b8860b,stroke-width:2px;"}};

    static std::unordered_map<std::string, int> class_defs_used;
    auto it = class_defs.find(hint);
    if (it == class_defs.end()) {
      return;
    }
    if (class_defs_used[hint] == 0) {
      class_defs_used[hint] = class_defs_used[hint] + 1;
      str << "    "
          << "classDef " << hint << " " << it->second << ";" << std::endl;
    }
    str << "    "
        << "class " << cxx_name << " " << hint << ";" << std::endl;
    return;
  }

public:
  const NodeInput node_input_;
  const std::string hint_;
  const std::shared_ptr<Pattern> pattern_;
  const std::string cxx_name_;
  const std::vector<std::shared_ptr<NodePattern>> inputs_;
};

struct NodePatternBuiler {
public:
  /**
   * Builds a collection of node patterns from a vector of nodes.
   *
   * @param nodes A vector of pointers to nodes.
   * @return A shared pointer to the last node pattern in the collection.
   */
  std::shared_ptr<NodePattern>
  build_nodes(std::vector<vaip_cxx::NodeConstRef> nodes) {
    for (auto node : nodes) {
      build_node(node);
    }
    return vector_node_patterns_.back();
  }
  /**
   * Builds a node pattern based on the given node.
   *
   * @param node The node to build the pattern for.
   * @return A shared pointer to the built node pattern.
   */
  std::shared_ptr<NodePattern> build_node(const Node& node) {
    auto hint = get_hint(node);
    auto cxx_name = new_unique_name(hint);
    auto node_inputs = node_get_inputs(node);
    auto& node_arg = node_get_first_output_node_arg(node);
    auto ni_name = node_arg_get_name(node_arg);
    auto pattern_args = std::vector<std::shared_ptr<Pattern>>{};
    auto node_pattern_inputs = std::vector<std::shared_ptr<NodePattern>>{};
    for (auto ni : node_inputs) {
      auto ni_name = node_arg_get_name(*ni.node_arg);
      auto it = map_node_patterns_.find(ni_name);
      auto arg_node_pattern = std::shared_ptr<NodePattern>();
      if (it != map_node_patterns_.end()) {
        if (ni.node == nullptr) {
          if (ENV_PARAM(ENABLE_CONSTANT_SHARING)) {
            arg_node_pattern = it->second;
          } else {
            arg_node_pattern = build_constant_pattern(*ni.node_arg);
          }
        } else {
          arg_node_pattern = it->second;
        }
      } else {
        if (ni.node == nullptr) {
          arg_node_pattern = build_constant_pattern(*ni.node_arg);
        } else {
          arg_node_pattern = build_input_pattern(*ni.node_arg);
        }
      }
      pattern_args.push_back(arg_node_pattern->pattern());
      node_pattern_inputs.push_back(arg_node_pattern);
    }
    auto op_type = node_op_type(node);
    auto op_domain = node_op_domain(node);
    auto pattern_op_type = std::string();
    if (node_op_domain(node).empty()) {
      pattern_op_type = op_type;
    } else {
      pattern_op_type = op_domain + ":" + op_type;
    }
    auto pattern = builder_.node2(pattern_op_type, pattern_args);
    auto& node_arg_name = node_arg_get_name(node_arg);
    builder_.bind(node_arg_name, pattern);
    return push_back_build_node_pattern({&node, &node_arg}, hint, pattern,
                                        cxx_name, node_pattern_inputs);
  }
  /**
   * Builds a constant pattern for the given NodeArg.
   *
   * @param node_arg The NodeArg for which the constant pattern is built.
   * @return A shared pointer to the built NodePattern.
   */
  std::shared_ptr<NodePattern> build_constant_pattern(const NodeArg& node_arg) {
    auto& node_arg_name = node_arg_get_name(node_arg);
    LOG(INFO) << "create constant for " << node_arg_name;
    if (ENV_PARAM(ENABLE_CONSTANT_SHARING)) {
      auto it = map_node_patterns_.find(node_arg_name);
      if (it != map_node_patterns_.end()) {
        LOG(INFO) << "re-use constant " << it->second->pattern()->get_id();
        return it->second;
      }
    }
    auto pattern = builder_.constant();
    builder_.bind(node_arg_name, pattern);
    auto hint = std::string("constant");
    auto cxx_name = new_unique_name(hint);
    LOG(INFO) << "create a new constant " << pattern->get_id();
    return push_back_build_node_pattern(
        {nullptr, &node_arg}, hint, pattern, cxx_name,
        std::vector<std::shared_ptr<NodePattern>>{});
  }

  /**
   * Builds a node pattern for the given input node argument.
   *
   * @param node_arg The input node argument.
   * @return A shared pointer to the built node pattern.
   */
  std::shared_ptr<NodePattern> build_input_pattern(const NodeArg& node_arg) {
    auto& node_arg_name = node_arg_get_name(node_arg);
    auto it = map_node_patterns_.find(node_arg_name);
    if (it != map_node_patterns_.end()) {
      return it->second;
    }
    auto pattern = builder_.wildcard();
    builder_.bind(node_arg_name, pattern);
    auto hint = std::string("input");
    auto cxx_name = new_unique_name(hint);
    return push_back_build_node_pattern(
        {nullptr, &node_arg}, hint, pattern, cxx_name,
        std::vector<std::shared_ptr<NodePattern>>{});
  }

  /**
   * @brief Adds a new node pattern to the vector_node_patterns_ and
   * map_node_patterns_.
   *
   * This function takes in the node_input, hint, pattern, cxx_name, and inputs
   * parameters and creates a new NodePattern object using these parameters. The
   * newly created NodePattern object is then added to the vector_node_patterns_
   * and map_node_patterns_ containers.
   *
   * @param node_input The NodeInput object.
   * @param hint The hint string.
   * @param pattern The shared pointer to the Pattern object.
   * @param cxx_name The cxx_name string.
   * @param inputs The vector of shared pointers to NodePattern objects.
   */
  std::shared_ptr<NodePattern> push_back_build_node_pattern(
      const NodeInput& node_input, const std::string& hint,
      const std::shared_ptr<Pattern> pattern, const std::string& cxx_name,
      const std::vector<std::shared_ptr<NodePattern>>& inputs) {
    auto& node_arg_name = node_arg_get_name(*node_input.node_arg);
    auto ret = std::make_shared<NodePattern>(node_input, hint, pattern,
                                             cxx_name, inputs);
    vector_node_patterns_.push_back(ret);
    map_node_patterns_[node_arg_name] = ret;
    return ret;
  }
  /**
   * Returns a hint for the given node.
   *
   * The hint is generated based on the node's operation type and domain.
   * If the node's domain is empty, the hint is simply the operation type.
   * Otherwise, the hint is a combination of the domain and operation type,
   * separated by an underscore. hint is used to generate c++ variable name and
   * mmd node id
   *
   * @param node The node for which to generate the hint.
   * @return The hint for the given node.
   */
  std::string get_hint(const Node& node) {
    auto to_cxx_id = [](const std::string& v_name) -> std::string {
      auto name = v_name;
      std::transform(
          name.begin(), name.end(), name.begin(),
          [](std::string::value_type c) { return std::isalnum(c) ? c : '_'; });
      return name;
    };
    auto op_type = node_op_type(node);
    if (node_op_domain(node).empty()) {
      return op_type;
    } else {
      return to_cxx_id(node_op_domain(node)) + "_" + op_type;
    }
  }

  /**
   * Generates a new unique name based on the given hint.
   *
   * @param hint The hint for the new unique name.
   * @return The new unique name.
   */
  std::string new_unique_name(const std::string& hint) {
    auto c = name_counter_[hint];
    auto ret = hint + "_" + std::to_string(c);
    name_counter_[hint] = c + 1;
    return ret;
  };
  /**
   * @brief Tests the pattern matching algorithm on a given graph.
   *
   * This function takes a graph and an output node as input and performs
   * pattern matching on the graph using the root pattern obtained from the
   * output node. It prints the matching pattern and returns the result of the
   * matching process.
   *
   * @param graph The graph to perform pattern matching on.
   * @param output The output node of the graph to start pattern matching from.
   */
  void test(vaip_cxx::GraphConstRef graph,
            const std::vector<std::string>& outputs) {
    auto patterns = std::vector<std::shared_ptr<vaip_core::Pattern>>{};
    patterns.reserve(outputs.size());
    auto first_node = std::optional<vaip_cxx::NodeConstRef>();
    for (auto output : outputs) {
      auto node = graph.find_node(output);
      CHECK(node != std::nullopt)
          << "- Error: Cannot find the root pattern "
             "-------------------------------------------"
          << std::endl;
      if (first_node == std::nullopt) {
        first_node = node;
      }
      // Get root pattern
      auto root_pattern = builder_.get_pattern(output);
      CHECK(root_pattern != nullptr)
          << "- Error: Cannot find the root pattern "
             "-------------------------------------------"
          << std::endl;

      std::cout << "- Matching pattern "
                   "---------------------------------------------------"
                << std::endl;
      vaip_core::Pattern::enable_trace(1);
      auto match = root_pattern->match(graph, *node);
      patterns.push_back(root_pattern);
      CHECK(match != nullptr)
          << "- Error: Cannot match the root pattern " << std::endl;
      std::cout << "---------------------------------------------------"
                << std::endl;
      std::cout << "- Pattern matched ?: " << (match ? "true" : "false") << "\n"
                << "- Pattern: " << root_pattern->debug_string() << "\n"
                << "- Node: " << *node << std::endl;
      std::cout << "---------------------------------------------------"
                << std::endl;
    }
    if (patterns.size() > 1) {
      std::cout << "- Test Multiple output patterns: " << std::endl;
      auto seq = builder_.sequence(patterns);
      auto match = seq->match(graph, *first_node);
      CHECK(match != nullptr)
          << "- Error: Cannot match the sequence pattern " << std::endl;
    }
    return;
  }
  /**
   * Generates C++ code based on the provided inputs and writes it to a file.
   *
   * @param opt_inputs A vector of strings representing the optional inputs.
   * @param opt_output A string representing the optional output.
   * @param opt_onnx_file A string representing the optional ONNX file.
   * @param opt_cxx_file A string representing the optional C++ file to write
   * the generated code to.
   */
  void generate_cxx_code(const std::vector<std::string>& opt_inputs,
                         const std::vector<std::string>& opt_outputs,
                         const std::string opt_onnx_file,
                         const std::string& opt_cxx_file) const {
    std::ostringstream cxx_src_stream;
    cxx_src_stream                                                           //
        << "/** generated by the following command:\n"
        << "env \\\n"                                                        //
        << " IGNORE_CONSTANT=" << ENV_PARAM(IGNORE_CONSTANT) << " \\\n"      //
        << " ENABLE_CONSTNAT_SHARING=" << ENV_PARAM(ENABLE_CONSTANT_SHARING) //
        << " \\\n"                                                           //
        << " $BUILD/vaip/onnxruntime_vitisai_ep/onnx_pattern_gen \\\n";
    for (auto& input : opt_inputs) {
      cxx_src_stream << " -i " << input << " \\\n";
    }
    for (auto& output : opt_outputs) {
      cxx_src_stream << " -o " << output << " \\\n";
    }
    cxx_src_stream << " -f " << opt_onnx_file << "\\\n";
    cxx_src_stream << " -c " << opt_cxx_file << "\n";
    cxx_src_stream << "*/\n";
    for (auto np : vector_node_patterns_) {
      cxx_src_stream << "auto " << np->cxx_name_ << " = "
                     << np->cxx_build_self() << "; // "
                     << " id = " << np->pattern_->get_id() << " "
                     << " node_arg_name = " << np->node_arg_name() << "\n"
                     << "builder.bind(\"" << np->node_arg_name() << "\","
                     << np->cxx_name_ << ");\n";
    }
    if (opt_outputs.size() == 1) {
      cxx_src_stream << "ret = " << vector_node_patterns_.back()->cxx_name_
                     << ";" << std::endl;
    } else {
      cxx_src_stream << "ret = "
                        "builder.sequence(std::vector<std::shared_ptr<vaip_"
                        "core::Pattern>>{";
      for (auto output : opt_outputs) {
        cxx_src_stream << map_node_patterns_.at(output)->cxx_name_ << ",";
      }
      cxx_src_stream << "});";
    }

    auto code = cxx_src_stream.str();
    // dump the file
    auto inc = std::filesystem::path(__FILE__).parent_path() / opt_cxx_file;
    CHECK(std::ofstream(inc).write(code.data(), code.size()).good())
        << " failed to write to " << inc;
    LOG(INFO) << "write generated c++ code to " << inc;
  }

  /**
   * Generates a Mermaid code for a flowchart based on the given input
   * parameters and writes it to a file.
   *
   * @param opt_inputs A vector of strings representing the optional inputs.
   * @param opt_output A string representing the optional output.
   * @param opt_onnx_file A string representing the optional ONNX file.
   * @param opt_mmd_file A string representing the path to the output Mermaid
   * file.
   */
  void generate_mmd_code(const std::vector<std::string>& opt_inputs,
                         const std::vector<std::string>& opt_outputs,
                         const std::string opt_onnx_file,
                         const std::string& opt_mmd_file) const {
    std::ostringstream mmd_stream;
    mmd_stream << "flowchart TB" << std::endl;
    for (auto np : vector_node_patterns_) {
      mmd_stream << np->mmd_build_self(opt_outputs);
    }
    auto code = mmd_stream.str();
    // dump the file
    auto inc = std::filesystem::path(__FILE__).parent_path() / opt_mmd_file;
    CHECK(std::ofstream(inc).write(code.data(), code.size()).good())
        << " failed to write to " << inc;
    LOG(INFO) << "write generated mermaid diagram to " << inc;
  }

private:
  PatternBuilder builder_;
  std::unordered_map<std::string, int> name_counter_;
  std::unordered_map<std::string, std::shared_ptr<NodePattern>>
      map_node_patterns_;
  std::vector<std::shared_ptr<NodePattern>> vector_node_patterns_;
};
} // namespace

/**
 * Retrieves a vector of nodes from the given graph based on the specified
 * criteria.
 *
 * @param graph The graph to search for nodes.
 * @param to_ops A vector of strings representing the names of the target nodes
 * to search for. If empty, all output nodes in the graph will be considered as
 * target nodes.
 * @param from_op_names A vector of strings representing the names of the source
 * nodes to search from.
 * @return A vector of const Node pointers representing the nodes that satisfy
 * the search criteria.
 */
template <typename T>
std::vector<vaip_cxx::NodeConstRef> get_nodes(vaip_cxx::GraphConstRef graph,
                                              const T& node_output_names) {
  std::vector<vaip_cxx::NodeConstRef> nodes;
  nodes.reserve(node_output_names.size());
  for (auto it = node_output_names.rbegin(); it != node_output_names.rend();
       ++it) {
    const auto& name = *it;
    auto node = graph.find_node(name);
    if (node == std::nullopt) {
      LOG(FATAL) << "cannot find node:" << name;
    }
    nodes.push_back(*node);
  }
  std::sort(nodes.begin(), nodes.end(),
            [](const auto& a, const auto& b) { return a.index() < b.index(); });
  return nodes;
}
static void usage(const char* programName) {
  std::cout << "Usage: " << programName << " [options]\n"
            << "Options:\n"
            << " -i <input>   Specify input of an onnx model\n"
            << " -o <output>  Specify output of an onnx model\n"
            << " -f <file>    Specify file of an onnx model\n"
            << " -c <file>    Specify C++ output file\n"
            << " -m <file>    Specify mermaid output file\n"
            << " -h           Display this help message\n";
}

// example usage:

// clang-format off
// $BUILD/vaip/onnxruntime_vitisai_ep/onnx_pattern_gen -i 38 -o 62 -f $BUILD/../vaip_regression/5/Resnet18_int.onnx
// clang-format on
int main(int argc, char* argv[]) {
  try {
    auto opt_onnx_file = std::string();
    int opt = 0;
    auto opt_cxx_output_file = std::string("onnx_grep_cxx_pattern.h.inc");
    auto opt_mmd_output_file = std::string("onnx_grep_cxx_pattern.mmd");
    auto opt_inputs = std::vector<std::string>{};
    auto opt_outputs = std::vector<std::string>{};
    while ((opt = getopt(argc, argv, "f:i:o:c:m:h")) != -1) {
      switch (opt) {
      case 'i': {
        opt_inputs.push_back(std::string(optarg));
        break;
      }
      case 'o': {
        opt_outputs.push_back(std::string(optarg));
        break;
      }
      case 'f': {
        opt_onnx_file = std::string(optarg);
        break;
      }
      case 'c': {
        opt_cxx_output_file = std::string(optarg);
        break;
      }
      case 'm': {
        opt_mmd_output_file = std::string(optarg);
        break;
      }
      case 'h': {
        usage(argv[0]);
        exit(0);
      }
      }
    }
    // intialize the main function.
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "onnx_pattern_gen");
    initialize_vaip();

    // Check command line args
    CHECK_NE(opt_onnx_file, "")
        << " -f <model.onnx> is required. " << opt_onnx_file;
    CHECK_NE(opt_inputs.size(), 0) << " -i <input> is required";
    CHECK_GE(opt_outputs.size(), 1) << " -o <output> is required";

    // Load model
    auto model = vaip_cxx::Model::load(opt_onnx_file);
    auto graph = model->main_graph();
    graph.resolve();
    auto [meta_def, error] = graph.try_fuse("onnx_pattern_gen", opt_inputs,
                                            opt_outputs, {}, "no_device");

    if (meta_def == nullptr) {
      LOG(FATAL) << "try_fuse error: " << error.comments;
    }

    /// get all nodes in the subgraph.
    auto nodes = get_nodes(graph, meta_def->nodes());
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "- Found subgraph as below: "
                 "-----------------------------------------"
              << std::endl;
    std::cout << meta_def->DebugString() << std::endl;
    std::cout << std::endl;
    std::cout << "- Generate pattern "
                 "-----------------------------------------"
              << std::endl;
    auto builder = NodePatternBuiler();
    auto node_pattern = builder.build_nodes(nodes);
    builder.test(graph, opt_outputs);
    builder.generate_cxx_code(opt_inputs, opt_outputs, opt_onnx_file,
                              opt_cxx_output_file);
    builder.generate_mmd_code(opt_inputs, opt_outputs, opt_onnx_file,
                              opt_mmd_output_file);
  } catch (const std::exception& e) {
    std::cerr << "exception occurs : " << e.what() << "\n";
  }

  return 0;
}

#include "./getopt.c"
