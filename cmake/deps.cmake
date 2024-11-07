#
# The Xilinx Vitis AI Vaip in this distribution are provided under the following
# free and permissive binary-only license, but are not provided in source code
# form.  While the following free and permissive license is similar to the BSD
# open source license, it is NOT the BSD open source license nor other
# OSI-approved open source license.
#
# Copyright (C) 2022 Xilinx, Inc. All rights reserved.
# Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
#
# Redistribution and use in binary form only, without modification, is permitted
# provided that the following conditions are met:
#
# 1. Redistributions must reproduce the above copyright notice, this list of
#   conditions and the following disclaimer in the documentation and/or other
#   materials provided with the distribution.
#
# 1. The name of Xilinx, Inc. may not be used to endorse or promote products
#   redistributed with this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY XILINX, INC. "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
# EVENT SHALL XILINX, INC. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE
#
include(FetchContent)
file(STRINGS ${CMAKE_CURRENT_LIST_DIR}/deps.txt VAIP_DEPS_LIST)
file(READ "${CMAKE_CURRENT_LIST_DIR}/dep.h.inc.in" VAIP_DEP_H_INC_IN)
set(VAIP_DEP_H_INC "")
foreach(VAIP_DEP IN LISTS VAIP_DEPS_LIST)
  message("VAIP_DEP = ${VAIP_DEP}")
  # Lines start with "#" are comments
  if(NOT VAIP_DEP MATCHES "^#")
    # The first column is name
    list(POP_FRONT VAIP_DEP VAIP_DEP_NAME)
    # The second column is URL
    # The URL below may be a local file path or an HTTPS URL
    list(POP_FRONT VAIP_DEP VAIP_DEP_URL)
    set(DEP_URL_${VAIP_DEP_NAME} ${VAIP_DEP_URL})
    # The third column is SHA1 hash value
    set(DEP_SHA1_${VAIP_DEP_NAME} ${VAIP_DEP})
    string(CONFIGURE "${VAIP_DEP_H_INC_IN}" _tmp @ONLY)
    string(APPEND VAIP_DEP_H_INC "${_tmp}")
  endif()
endforeach()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/vaip_deps.inc.h" "${VAIP_DEP_H_INC}")
FetchContent_Declare(
  GSL
  URL ${DEP_URL_microsoft_gsl}
  URL_HASH SHA1=${DEP_SHA1_microsoft_gsl}
  FIND_PACKAGE_ARGS 4.0 NAMES GSL
)
FetchContent_MakeAvailable(GSL)

set(WITH_GFLAGS OFF)
FetchContent_Declare(
  glog
  URL ${DEP_URL_glog}
  URL_HASH SHA1=${DEP_SHA1_glog}
  OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
  Boost
  URL ${DEP_URL_Boost}
  URL_MD5 ${DEP_SHA1_Boost}
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  CMAKE_ARGS -DBOOST_EXCLUDE_LIBRARIES=mp11,headers
  OVERRIDE_FIND_PACKAGE
)
set(BOOST_INCLUDE_LIBRARIES config filesystem system graph interprocess)
set(BOOST_EXCLUDE_LIBRARIES mp11 headers)
set(Boost_VERBOSE TRUE)
set(Boost_DEBUG TRUE)
FetchContent_Declare(
  unilog
  GIT_REPOSITORY ${DEP_URL_unilog}
  GIT_TAG ${DEP_SHA1_unilog}
  GIT_SHALLOW FALSE
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  xir
  GIT_REPOSITORY ${DEP_URL_xir}
  GIT_TAG ${DEP_SHA1_xir}
  GIT_SHALLOW FALSE
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  target-factory
  GIT_REPOSITORY ${DEP_URL_target_factory}
  GIT_TAG ${DEP_SHA1_target_factory}
  GIT_SHALLOW FALSE
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  vart
  GIT_REPOSITORY ${DEP_URL_vart}
  GIT_TAG ${DEP_SHA1_vart}
  GIT_SHALLOW FALSE
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  trace-logging
  GIT_REPOSITORY ${DEP_URL_trace_logging}
  GIT_TAG ${DEP_SHA1_trace_logging}
  GIT_SHALLOW FALSE
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  graph-engine
  GIT_REPOSITORY ${DEP_URL_graph_engine}
  GIT_TAG ${DEP_SHA1_graph_engine}
  GIT_SHALLOW FALSE
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  Eigen3
  URL ${DEP_URL_eigen}
  URL_HASH SHA1=${DEP_SHA1_eigen}
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  ZLIB
  GIT_REPOSITORY ${DEP_URL_zlib}
  GIT_TAG ${DEP_SHA1_zlib}
  GIT_SHALLOW TRUE
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY ${DEP_URL_spdlog}
  GIT_TAG ${DEP_SHA1_spdlog}
  GIT_SHALLOW TRUE
  CMAKE_ARGS -DWITH_GFLAGS=OFF -DWITH_GTEST=OFF
  OVERRIDE_FIND_PACKAGE
)
#FetchContent_Declare(
#  GTest
#  GIT_REPOSITORY ${DEP_URL_GTest}
#  GIT_TAG ${DEP_SHA1_GTest}
#  GIT_SHALLOW TRUE
#  CMAKE_ARGS -Dgtest_force_shared_crt=ON
#  OVERRIDE_FIND_PACKAGE
#)
FetchContent_Declare(
  dod
  GIT_REPOSITORY ${DEP_URL_dod}
  GIT_TAG ${DEP_SHA1_dod}
  GIT_SHALLOW FALSE
  CMAKE_ARGS -DBUILD_TEST=OFF -DDISABLE_LARGE_TXN_OPS=ON
  OVERRIDE_FIND_PACKAGE
)
FetchContent_Declare(
  transformers
  GIT_REPOSITORY ${DEP_URL_transformers}
  GIT_TAG ${DEP_SHA1_transformers}
  GIT_SHALLOW FALSE
)
FetchContent_Populate(transformers)
FetchContent_Declare(
  xaiengine
  GIT_REPOSITORY ${DEP_URL_xaiengine}
  GIT_TAG ${DEP_SHA1_xaiengine}
  SOURCE_SUBDIR driver/src
  GIT_SHALLOW FALSE
  OVERRIDE_FIND_PACKAGE
)
set(XAIENGINE_BUILD_SHARED OFF CACHE INTERNAL "We want static library")

## configurations
set(WITH_XCOMPILER ON CACHE BOOL "enable XCOMPILER")
set(WITH_OPENSSL OFF CACHE BOOL "enable open ssl")
set(WITH_CPURUNNER OFF CACHE BOOL "enable cpu runner")
set(BUILD_PYTHON_EXT OFF CACHE BOOL "enable python ext")
set(EN_LLM_DOD_OPS ON CACHE BOOL "enable dd flow")
set(EN_VAIML ON CACHE BOOL "enable vaiml flow")
set(ENABLE_VITIS_AI_CUSTOM_OP OFF "enable vitis ai custom op")
set(PACK_XCLBIN_PATH "" CACHE STRING "list of xclbin files")
set(ENABLE_BUILD_VOE_WHEEL OFF CACHE BOOL "internal used only" FORCE)
set(INSTALL_USER ON CACHE BOOL "internal used only" FORCE)
set(ENABLE_XRT_SHARED_CONTEXT ON CACHE BOOL "internal used only" FORCE)
## make them available.
find_package(unilog)
find_package(Protobuf)
find_package(xir)
add_library(xir::xir ALIAS xir)
find_package(target-factory)
add_library(target-factory::target-factory ALIAS target-factory)
find_package(XRT)
find_package(vart)
if(WITH_CPURUNNER)
  add_library(vart::cpu-runner ALIAS cpu-runner)
endif(WITH_CPURUNNER)
find_package(trace-logging)
find_package(graph-engine)
add_library(graph-engine::graph-engine ALIAS graph-engine)
find_package(Eigen3)
find_package(ZLIB)
if(NOT TARGET ZLIB::ZLIB)
  add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()
find_package(vaip)
if(MSVC)
  find_package(xaiengine)
  add_library(xaiengine::xaiengine ALIAS xaiengine)
endif(MSVC)

foreach(_target
    boost_filesystem
    boost_system
    boost_graph
    xaiengine
    dd_metastate_proto
    dummy-runner
    cpu-runner
    generate_test_cases
    generator_all_xir_op_names
    glog
    glogbase
    graph-engine
    mem-manager
    onnx_dump_txt
    onnx_grep
    onnx_knife
    onnx_pattern_gen
    onnx2xir
    runner
    target_list.hpp
    target-factory
    test_dummy_runner_simple
    test_encryption
    test_load_context
    trace
    trace-logging
    unilog
    util
    voe_py_pass
    xir
    xir_util
    zlib
    zlibstatic
    example
    minigzip
  )
  if(TARGET ${_target})
    set_target_properties(${_target} PROPERTIES FOLDER "VAIP")
  endif()
endforeach()

find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(dod)
if(TARGET dyn_dispatch_core)
  set_property(TARGET dyn_dispatch_core PROPERTY COMPILE_WARNING_AS_ERROR OFF)
  message(WARNING "ignore compilation errors for DOD")
endif(TARGET dyn_dispatch_core)
execute_process(
  COMMAND
  ${CMAKE_COMMAND} -E make_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/vaip/include/ryzenai/dynamic_dispatch
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory
    ${dod_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vaip/include/ryzenai/dynamic_dispatch
  COMMAND_ERROR_IS_FATAL ANY)
