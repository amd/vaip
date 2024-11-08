#
# The Xilinx Vitis AI Vaip in this distribution are provided under the following
# free and permissive binary-only license, but are not provided in source code
# form.  While the following free and permissive license is similar to the BSD
# open source license, it is NOT the BSD open source license nor other
# OSI-approved open source license.
#
# Copyright (C) 2022 Xilinx, Inc.
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

cmake_minimum_required(VERSION 3.17)
project(
  vaip
  VERSION 1.0.0
  LANGUAGES C CXX)

find_package(unilog)

vai_add_library(NAME pass_py_ext INCLUDE_DIR include SRCS src/pass_main.cpp)

if(CMAKE_CROSSCOMPILING)
  set_property(
    TARGET pass_py_ext
    APPEND
    PROPERTY INCLUDE_DIRECTORIES ${VAI_PYTHON_INCLUDE_DIRS})
  target_link_libraries(pass_py_ext PRIVATE -l${VAI_PYTHON_LIB})
else(CMAKE_CROSSCOMPILING)
  target_link_libraries(pass_py_ext PRIVATE ${PYTHON_LIBRARIES})
  set_property(
    TARGET pass_py_ext
    APPEND
    PROPERTY INCLUDE_DIRECTORIES "${PYTHON_INCLUDE_DIRS}")
endif(CMAKE_CROSSCOMPILING)

target_link_libraries(pass_py_ext PRIVATE vaip::core vart::util glog::glog
                                          pybind11::embed)



set(voe_sources
  src/common.hpp
  src/vaip_pass_py_ext_py_module.cpp
  src/util.cpp
  src/util.hpp
)
if(USE_STATIC_PYTHON)
  pybind11_add_module(
    voe_cpp2py_export
    STATIC
    ${voe_sources}
  )
else()
  vai_add_pybind11_module(
  voe_cpp2py_export
  PACKAGE_NAME
  voe
  ${voe_sources})
endif()

target_link_libraries(voe_cpp2py_export PRIVATE vaip::core vart::util
                                                glog::glog)
if (USE_STATIC_PYTHON)
  install(
    TARGETS voe_cpp2py_export
    EXPORT voe_cpp2py_export-targets
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
  install(
    EXPORT voe_cpp2py_export-targets
    NAMESPACE vaip::
    COMPONENT base
    DESTINATION share/cmake/vaip)
    file(APPEND ${CMAKE_BINARY_DIR}/components.txt "voe_cpp2py_export\n")
endif()