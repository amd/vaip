/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
#pragma once
#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wpedantic"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wsign-compare"
#  pragma GCC diagnostic ignored "-Wunused-variable"
#  pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#define ORT_API_MANUAL_INIT
#include <onnxruntime_c_api.h>
#include <onnxruntime_cxx_api.h>
#include <onnxruntime_lite_custom_op.h>
