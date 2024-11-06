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

#pragma once
#include "./_sanity_check.hpp"
#include "./node_attr.hpp"
#include "vaip/node.hpp"
#include "vaip/node_arg.hpp"
#include <glog/logging.h>
#include <type_traits>
#include <vaip/my_ort.h>
#include <vaip/vaip_ort_api.h>
#ifdef _WIN32
#  pragma warning(push, 0)
#endif
#include "./pass_context.hpp"
#ifdef _WIN32
#  pragma warning(pop)
#endif
#include <filesystem>
#include <memory>
#include <string>

namespace vaip_core {
#define PASS_LOG(self_, n)                                                     \
  LOG_IF(INFO, (self_.get_pass_proto().enable_log() &&                         \
                self_.get_pass_proto().log_verbosity() >= n))
using namespace onnxruntime;

/** @brief create a Context object from a cache directory.
 *
 * it is mainly used by some internal tests and tools.
 *
 */

VAIP_DLL_SPEC std::shared_ptr<PassContext>
load_context(const std::filesystem::path& cache_dir);

/** @brief For troubleshooting Pass:fuse / Pass::try_fuse error.
 *
 *  @sa Pass::fuse Pass::try_fuse
 */
struct TryFuseError {
  std::string comments;
  std::vector<std::string> path;
  std::vector<const Node*> body;
  std::vector<std::string> arguments;
  std::vector<std::string> return_values;
};

/** @brief the base class for all passes.
 *
 */
class IPass {
public:
  /** @brief create a concrete pass object from `PassProto`
   *
   *  @param context the Context object shared among all passes.
   *  @param pass_proto pass configuration, pass_proto.plugin is the
   *  name of the shared library, e.g "vaip-pass_merge_fix", see
   *  `vaip_config.json` for more examples.
   *
   */
  VAIP_DLL_SPEC static std::unique_ptr<IPass>
  create_pass(std::shared_ptr<PassContext> context,
              const PassProto& pass_proto);

  /** @brief create concrete passes object from `PassProto`
   *
   *  @param context the Context object shared among all passes.
   *  @param pass_proto
   *
   *  repeatedly invoke Pass::create_pass to create a vector of IPass objects.
   */
  static std::vector<std::shared_ptr<IPass>>
  create_passes(std::shared_ptr<PassContext> context,
                const google::protobuf::RepeatedPtrField<PassProto>& passes);
  /** @brief create a concrete pass object from `PassInfo`
   *
   *  @param context the Context object shared among all passes.
   *  @param pass_info
   *
   * @NOTE only used for internal test purpuse.
   */
  VAIP_DLL_SPEC static std::unique_ptr<IPass>
  create_pass(std::shared_ptr<PassContext> context,
              const struct PassInfo& pass_info);

  /** @brief apply all passes
   *
   * @param passes
   * @param graph
   *
   * `graph` is modified in place by all these passes in sequence.
   *
   */
  VAIP_DLL_SPEC static void
  run_passes(std::vector<std::shared_ptr<IPass>> passes, Graph& graph);

  using action_t = std::function<void(IPass& self, Graph& graph)>;
  using node_action_t =
      std::function<bool(IPass& self, Graph& graph, const Node& node)>;

  IPass() = default;
  virtual ~IPass() = default;

public:
  virtual const std::string& name() const = 0;
  /** @brief do not use this function. internal use only
   */
  virtual void* get_state() = 0;
  virtual std::filesystem::path
  get_cache_file_name(const std::string& filename) const = 0;
  virtual const ConfigProto& get_config_proto() const = 0;
  virtual const std::filesystem::path& get_log_path() const = 0;

  /** @brief do not use this function. internal use only
   */
  virtual void set_fix_info(const char* name, int fix_pos) = 0;
  /** @brief do not use this function. internal use only
   */
  virtual int get_fix_info(const char* name) const = 0;
  /** @brief do not use this function. internal use only
   */
  virtual bool has_fix_info(const char* name) const = 0;
  /** @brief do not use this function. internal use only
   */
  virtual void add_subgraph_device_count(const std::string& device,
                                         int count) = 0;
  /** @brief do not use this function. internal use only
   */
  virtual void create_const(const char* name, gsl::span<const char> data,
                            const std::vector<int64_t>& shape, int type) = 0;
  /** @brief do not use this function. internal use only
   */
  inline void create_const(const Node& node, gsl::span<const char> data) {
    auto name = node_get_output_name(node);
    auto& arg = node_get_output_node_arg(node);
    auto shape = node_arg_get_shape_i64(arg);
    CHECK(shape != nullptr) << node_arg_as_string(arg) << " shape absent";
    auto type = VAIP_ORT_API(node_arg_get_element_type)(arg);
    create_const(name.c_str(), data, *shape, type);
  }
  /** @brief do not use this function. internal use only
   */
  virtual void create_empty_const(const char* name, size_t size,
                                  const std::vector<int64_t>& shape,
                                  int type) = 0;
  /** @brief do not use this function. internal use only
   */
  inline void create_empty_const(const Node& node, size_t size) {
    auto name = node_get_output_name(node);
    auto& arg = node_get_output_node_arg(node);
    auto shape = node_arg_get_shape_i64(arg);
    CHECK(shape != nullptr) << node_arg_as_string(arg) << " shape absent";
    auto type = VAIP_ORT_API(node_arg_get_element_type)(arg);
    create_empty_const(name.c_str(), size, *shape, type);
  }
  /** @brief do not use this function. internal use only
   */
  virtual void
  create_lazy_const(const char* name, size_t size,
                    const std::vector<int64_t>& shape, int type,
                    const std::function<void(gsl::span<char>)>& lazy) = 0;
  /** @brief do not use this function. internal use only
   */
  inline void
  create_lazy_const(const Node& node, size_t size,
                    const std::function<void(gsl::span<char>)>& lazy) {
    auto& arg = node_get_output_node_arg(node);
    auto shape = node_arg_get_shape_i64(arg);
    CHECK(shape != nullptr) << node_arg_as_string(arg) << " shape absent";
    auto type = VAIP_ORT_API(node_arg_get_element_type)(arg);
    create_lazy_const(node_get_output_name(node).c_str(), size, *shape, type,
                      lazy);
  }
  /** @brief do not use this function. internal use only
   */
  virtual void create_const_alias(const char* alias_name, const char* name) = 0;
  /** @brief do not use this function. internal use only
   */
  inline void create_const_alias(const Node& new_node, const Node& origin) {
    auto alias = node_get_output_name(new_node);
    auto name = node_get_output_name(origin);
    create_const_alias(alias.c_str(), name.c_str());
  }
  /** @brief do not use this function. internal use only
   */
  virtual bool has_const(const char* name) const = 0;
  /** @brief do not use this function. internal use only
   */
  virtual ConstDataInfo get_const_info(const char* name) const = 0;
  // eval lazy function after we invoke get_const_data_ptr.
  /** @brief do not use this function. internal use only
   */
  virtual void* get_const_data_ptr(const char* name, bool force) const = 0;
  template <typename T> inline gsl::span<T> get_const_data(const char* name) {
    auto info = get_const_info(name);
    auto ptr = get_const_data_ptr(name, true /*force*/);
    return gsl::span<T>(reinterpret_cast<T*>(ptr), info.size() / sizeof(T));
  }
  /** @brief do not use this function. internal use only
   */
  template <typename T> inline gsl::span<T> get_const_data(const Node& node) {
    auto name = node_get_output_name(node);
    return get_const_data<T>(name.c_str());
  }
  /** @brief do not use this function. internal use only
   */
  inline std::vector<int64_t> get_const_data_shape(const Node& node) {
    auto name = node_get_output_name(node);
    auto info = get_const_info(name.c_str());
    return std::vector<int64_t>{info.shape().begin(), info.shape().end()};
  }
  /** @brief do not use this function. internal use only
   */
  template <typename T>
  inline gsl::span<T> get_const_data(const NodeArg& node_arg) {
    auto name = node_arg_get_name(node_arg);
    return get_const_data<T>(name.c_str());
  }
  /** @brief do not use this function. internal use only
   */
  template <typename T>
  inline std::vector<T> const_data_into(const NodeArg& node_arg);
  /** @brief do not use this function. internal use only
   */
  inline std::vector<int64_t> get_const_data_shape(const NodeArg& node_arg) {
    auto name = node_arg_get_name(node_arg);
    auto info = get_const_info(name.c_str());
    return std::vector<int64_t>{info.shape().begin(), info.shape().end()};
  }

  /** @brief do not use this function. internal use only
   */
  virtual void dump_fix_info(const char* name) const = 0;
  /** @brief do not use this function. internal use only
   */
  virtual void dump_const_info(const char* name) const = 0;
  /** @brief do not use this function. internal use only
   */
  virtual void dump_const_data(const char* name) const = 0;
  /** @brief do not use this function. internal use only
   */
  virtual const PassProto& get_pass_proto() const = 0;

  /** @brief do not use this function. internal use only
   */
  virtual std::vector<AttributeProtoPtr>&
  node_extra_attrs(const char* name) = 0;
  /** @brief do not use this function. internal use only
   */
  inline void node_add_extra_attr(const char* name, const NodeAttr& attr) {
    node_extra_attrs(name).push_back(attr_proto_clone(attr.get()));
  }

  /** @brief extract a subgraph into an onnx Node.
   *
   * @note graph is modified in place.
   */
  virtual const Node& fuse(Graph& graph, MetaDefProto&& meta_def) = 0;

  /** @brief extract a subgraph into an onnx Node.
   *
   * @note graph is modified in place.
   */
  virtual MetaDefProto&
  fuse(Graph& graph, const std::string& name, const std::string& op_type,
       const std::vector<size_t>& nodes, const std::vector<std::string>& inputs,
       const std::vector<std::string>& outputs,
       const std::vector<std::string>& constant_initializers,
       const std::string& device) = 0;

  /** @brief extract a subgraph into an onnx Node. level 2 fuse
   *
   * @note graph is modified in place. context.json is not updated.
   */

  virtual const Node& level_2_fuse(Graph& graph,
                                   const MetaDefProto& meta_def) = 0;

  /** @brief atempt extract a subgraph into an onnx Node
   *
   * @note no modification is made
   * @todo change `Graph& graph` to `const Graph& graph`
   */
  VAIP_DLL_SPEC std::pair<std::unique_ptr<MetaDefProto>, TryFuseError>
  try_fuse(const Graph& graph, const std::string& name,
           const std::vector<std::string>& inputs,
           const std::vector<std::string>& outputs,
           const std::vector<std::string>& constant_initializers,
           const std::string& device) const;

  /** @brief access the shared Context Object. readonly
   *
   */
  virtual const std::shared_ptr<PassContext> get_context() const = 0;

  /** @brief access the shared Context Object. readwrite.
   *
   * @note: try not to invoke this function as much as possible,
   * i.e. not to update the Context object.
   */
  virtual std::shared_ptr<PassContext> get_context() = 0;
  /** @brief do not use this function. internal use only
   */
  virtual void add_context_resource(const std::string& name,
                                    std::shared_ptr<void> resource) = 0;

public:
  VAIP_DLL_SPEC
  void copy_fix_info(const Node& from_node, const Node& to_node);
  VAIP_DLL_SPEC
  void copy_fix_info(const std::string& from, const std::string& to);
  VAIP_DLL_SPEC
  void copy_fix_info(const char* from, const char* to);
};
VAIP_DLL_SPEC std::pair<std::unique_ptr<MetaDefProto>, TryFuseError>
IPass_try_fuse(const Graph& graph, const std::string& name,
               const std::vector<std::string>& inputs,
               const std::vector<std::string>& outputs,
               const std::vector<std::string>& constant_initializers1,
               const std::string& device);
template <>
inline std::vector<int64_t>
IPass::const_data_into<int64_t>(const NodeArg& node_arg) {
  auto name = node_arg_get_name(node_arg);
  auto info = get_const_info(name.c_str());
  auto ret = std::vector<int64_t>();
  if (info.type() == ONNX_NAMESPACE::TensorProto_DataType_INT32) {
    auto v1 = get_const_data<int32_t>(name.c_str());
    ret.resize(v1.size());
    std::transform(v1.begin(), v1.end(), ret.begin(),
                   [](int32_t val) { return static_cast<int64_t>(val); });
  } else if (info.type() == ONNX_NAMESPACE::TensorProto_DataType_INT64) {
    auto v1 = get_const_data<int64_t>(name.c_str());
    ret.resize(v1.size());
    std::transform(v1.begin(), v1.end(), ret.begin(),
                   [](int64_t val) { return static_cast<int64_t>(val); });
  } else {
    LOG(FATAL) << "unknown type " << info.DebugString();
  }
  return ret;
}

struct PassInfo {
  typedef union {
    void* __p; // ease of union initialization;
    bool (*process_node)(IPass& self, Graph& graph, const Node& node);
    void (*process_graph)(IPass& self, Graph& graph);
  } process_t;
  typedef struct {
    int type;
    process_t proc;
  } process_def;
  void* (*init)(IPass& self);
  void (*deinit)(void*);
  typedef void (*preprocess_t)(void* state, IPass& self, Graph& graph);
  preprocess_t preprocess;
  typedef void (*postprocess_t)(void* state, IPass& self, Graph& graph);
  postprocess_t postprocess;
  size_t size;
#ifdef _WIN32
#  pragma warning(push)
#  pragma warning(disable : 4200)
#else
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wpedantic"
#endif
  process_def processes[];
#ifdef _WIN32
#  pragma warning(pop)
#else
#  pragma GCC diagnostic pop
#endif

  IPass::action_t get_action(size_t index) const;
};
template <typename T, class = void> struct has_preprocess_t {
  static constexpr PassInfo::preprocess_t preprocess = nullptr;
};

template <typename T>
struct has_preprocess_t<T,
                        std::void_t<decltype(std::declval<T*>()->preprocess(
                            std::declval<IPass&>(), std::declval<Graph&>()))>>
    : public std::true_type {
  static constexpr PassInfo::preprocess_t preprocess =
      [](void* self, IPass& pass, Graph& graph) {
        static_cast<T*>(self)->preprocess(pass, graph);
      };
};

template <typename T, class = void> struct has_postprocess_t {
  static constexpr PassInfo::postprocess_t postprocess = nullptr;
};

template <typename T>
struct has_postprocess_t<T,
                         std::void_t<decltype(std::declval<T*>()->postprocess(
                             std::declval<IPass&>(), std::declval<Graph&>()))>>
    : public std::true_type {
  static constexpr PassInfo::postprocess_t postprocess =
      [](void* self, IPass& pass, Graph& graph) {
        static_cast<T*>(self)->postprocess(pass, graph);
      };
};
template <typename Pass> struct CommonInitAndDeinit {
  static void* init(IPass& self) {
    auto state = new Pass(self);
    return (void*)state;
  }
  static void deinit(void* state) { delete static_cast<Pass*>(state); }
};

template <typename T, class = void> struct has_process_t {
  // not defined
  // static constexpr int type = -1;
  // static constexpr void* process = nullptr;
};

template <typename T>
struct has_process_t<T, std::void_t<decltype(std::declval<T*>()->process(
                            std::declval<IPass&>(), std::declval<Graph&>()))>>
    : public std::true_type {
  static constexpr int type = 0;
  static void process(IPass& self, Graph& graph) {
    static_cast<T*>(self.get_state())->process(self, graph);
  }
};

template <typename T>
struct has_process_t<
    T, std::enable_if_t<std::is_same_v<
           bool, decltype(std::declval<T*>()->process(
                     std::declval<IPass&>(), std::declval<Graph&>(),
                     std::declval<const Node&>()))>>> : public std::true_type {
  static constexpr int type = 1;
  static bool process(IPass& self, Graph& graph, const Node& node) {
    return static_cast<T*>(self.get_state())->process(self, graph, node);
  }
};

template <typename Pass> struct ProcessorPassInfo {
  static vaip_core::PassInfo* pass_info() { return &info; }
  static PassInfo info;
};

#ifndef _WIN32
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wpedantic"
#endif
template <typename Pass>
PassInfo ProcessorPassInfo<Pass>::info = {
    CommonInitAndDeinit<Pass>::init,
    CommonInitAndDeinit<Pass>::deinit,
    has_preprocess_t<Pass>::preprocess,
    has_postprocess_t<Pass>::postprocess,
    1,
    {
        {has_process_t<Pass>::type, {(void*)has_process_t<Pass>::process}},
    }};
#ifndef _WIN32
#  pragma GCC diagnostic pop
#endif

namespace fs = std::filesystem;
using namespace onnxruntime;

IPass::action_t
create_action_from_node_action(IPass::node_action_t node_action);
IPass::action_t create_xmodel_process_graph(IPass::action_t action);
} // namespace vaip_core

#ifndef _WIN32
#  define DEFINE_VAIP_PASS(cls, id)                                            \
    extern "C" VAIP_PASS_ENTRY vaip_core::PassInfo* vaip_pass_info() {         \
      return ProcessorPassInfo<cls>::pass_info();                              \
    }                                                                          \
    extern "C" {                                                               \
    void* /* a hook var*/ id##__hook = nullptr;                                \
    }
#else
#  include <vitis/ai/plugin.hpp>
#  define DEFINE_VAIP_PASS(cls, id)                                            \
    static vaip_core::PassInfo* vaip_pass_info() {                             \
      return ProcessorPassInfo<cls>::pass_info();                              \
    }                                                                          \
    namespace {                                                                \
    static vitis::ai::StaticPluginRegister                                     \
        __register(OUTPUT_NAME, "vaip_pass_info", (void*)&vaip_pass_info);     \
    }                                                                          \
    extern "C" {                                                               \
    void* /* a hook var*/ id##__hook = &__register;                            \
    }

#endif
