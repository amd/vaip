/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
#pragma once
#include "vaip/vaip.hpp"
#include "vitis/ai/env_config.hpp"
#include <cmath>
#include <functional>
#include <glog/logging.h>
#include <numeric>
using namespace vaip_core;

namespace vaip::dd::qmatmulcalc {

struct MatmulQDQParams {
  std::vector<int64_t> c0_coeffs;
  std::vector<int32_t> qdq_params;
  int32_t c1;
  int32_t c2;
  int32_t c3_coeff_scale;
  int64_t c2_coeff_prime;
  int64_t c3_coeff_scale_shift;
  int64_t shft_c2;
  int64_t matmul_shift;
};
struct MatmulQDQParams_3d {
  std::vector<std::vector<int64_t>> c0_coeffs;
  std::vector<int32_t> qdq_params;
  int32_t c1;
  int32_t c2;
  int32_t c3_coeff_scale;
  int64_t c2_coeff_prime;
  int64_t c3_coeff_scale_shift;
  int64_t shft_c2;
  int64_t matmul_shift;
};

std::vector<int32_t> calculate_add_qdq_params(float a_sc, uint16_t a_zp,
                                              float b_sc, uint16_t b_zp,
                                              float o_sc, uint16_t o_zp);
int32_t srs_int32_even_fast(int32_t inp, int shift);

std::pair<int32_t, int16_t>
find_closest_shifted_int32_with_max_shift(double float_val, int32_t max_value,
                                          int32_t max_shift_val);

std::pair<int32_t, int16_t> find_closest_shifted_int32(double float_val,
                                                       int32_t max_value);

std::pair<int32_t, int16_t> find_closest_shifted_int32_gap(double float_val,
                                                           int32_t max_value);

std::pair<int16_t, int16_t> find_closest_shifted_int16(double float_val,
                                                       int32_t max_value);
MatmulQDQParams calculate_matmuladd_qdq_params_uint8_uint8(
    const std::vector<std::vector<uint8_t>>& weights,
    const std::vector<uint16_t>& bias, float a_sc, uint16_t a_zp, float w_sc,
    uint16_t w_zp, float b_sc, uint16_t b_zp, float q_sc, uint16_t q_zp);

MatmulQDQParams calculate_matmuladd_qdq_params_uint16_uint8(
    const std::vector<std::vector<uint8_t>>& weights,
    const std::vector<uint16_t>& bias, float a_sc, uint16_t a_zp, float w_sc,
    uint16_t w_zp, float b_sc, uint16_t b_zp, float q_sc, uint16_t q_zp);

MatmulQDQParams calculate_matmul_qdq_params_uint8_uint8(
    const std::vector<std::vector<uint8_t>>& weights, float a_sc, uint16_t a_zp,
    float w_sc, uint16_t w_zp, float q_sc, uint16_t q_zp);

MatmulQDQParams calculate_matmul_qdq_params_uint16_uint8(
    const std::vector<std::vector<uint8_t>>& weights, float a_sc, uint16_t a_zp,
    float w_sc, uint16_t w_zp, float q_sc, uint16_t q_zp);

MatmulQDQParams_3d calculate_matmul_3d_qdq_params_uint16_uint8(
    const std::vector<std::vector<std::vector<uint8_t>>>& weights, float a_sc,
    uint16_t a_zp, float w_sc, uint16_t w_zp, float q_sc, uint16_t q_zp);

MatmulQDQParams calculate_matmuladd_qdq_params_uint8_uint8_b32(
    const std::vector<std::vector<uint8_t>>& weights,
    const std::vector<int32_t>& bias, float a_sc, uint16_t a_zp, float w_sc,
    uint16_t w_zp, float b_sc, uint16_t b_zp, float q_sc, uint16_t q_zp);

MatmulQDQParams calculate_matmuladd_qdq_params_uint16_uint8_b32(
    const std::vector<std::vector<uint8_t>>& weights,
    const std::vector<int32_t>& bias, float a_sc, uint16_t a_zp, float w_sc,
    uint16_t w_zp, float b_sc, uint16_t b_zp, float q_sc, uint16_t q_zp);

MatmulQDQParams calculate_matmuladd_qdq_params_uint8_uint8_b32(
    const std::vector<std::vector<uint8_t>>& weights,
    const std::vector<int32_t>& bias, float a_sc, uint16_t a_zp, float w_sc,
    uint16_t w_zp, float b_sc, uint16_t b_zp, float q_sc, uint16_t q_zp);

MatmulQDQParams calculate_matmuladd_qdq_params_uint16_uint8_b32(
    const std::vector<std::vector<uint8_t>>& weights,
    const std::vector<int32_t>& bias, float a_sc, uint16_t a_zp, float w_sc,
    uint16_t w_zp, float b_sc, uint16_t b_zp, float q_sc, uint16_t q_zp);

// Function to convert float to bfloat16
uint16_t float_to_bfloat16(float value);

std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>
calc_eltwise_coeff(float a_sc, uint16_t a_zp, float b_sc, uint16_t b_zp);

std::tuple<uint16_t, uint16_t> calc_lrn_coeff(float q_sc, uint16_t q_zp);

template <typename T> float dq(T data, float& scale, T zp) {
  return static_cast<float>(data - zp) * scale;
}

template <typename T>
std::vector<uint16_t> dq_vec_to_bf16(std::vector<T>& data, float& scale,
                                     T& zp) {
  std::vector<uint16_t> ret;
  for (auto q : data) {
    uint16_t dd = float_to_bfloat16(dq(q, scale, zp));
    ret.push_back(dd);
  }
  return ret;
}

std::tuple<std::vector<int64_t>, int32_t, int64_t, int64_t, int64_t, int64_t>
compute_qdq_coeff_matmul_bias(float a_dq_xscale, uint8_t a_dq_xzero_pt,
                              const std::vector<std::vector<uint8_t>>& weights,
                              float w_dq_xscale, uint8_t w_dq_xzero_pt,
                              const std::vector<uint16_t>& bias,
                              float b_dq_xscale, uint8_t b_dq_xzero_pt,
                              float a_q_yscale, uint8_t a_q_yzero_pt);

std::tuple<int64_t, int32_t, int32_t>
global_avg_pool_qdq(double a_sc, uint16_t a_zp, double b_sc, uint16_t b_zp);

std::vector<int64_t> grpb_qgprb_vec64_fill(std::vector<int64_t> bias,
                                           int64_t qk_qdq_c0,
                                           int64_t smv_qdq_c0);

std::vector<int32_t>
gprb_vec32_fill(const std::vector<int64_t>& coeff_grpb, float act_scale,
                int32_t act_zero_point, float wgt_scale, int32_t wgt_zero_point,
                const std::vector<uint16_t>& model_a, float model_a_scale,
                int32_t model_a_zp, uint16_t model_b, float model_b_scale,
                int32_t model_b_zp, uint16_t model_c, float model_c_scale,
                int32_t model_c_zp, int32_t is_grpb_int16);

std::tuple<int64_t, int32_t, int64_t, int32_t, int32_t, int32_t, int32_t>
qdq_act_matmul_uint8_uint8_cstm(float a_dq_xscale, int64_t a_dq_xzero_pt,
                                int64_t weights_in_ch, float w_dq_xscale,
                                int64_t w_dq_xzero_pt, float a_q_yscale,
                                int64_t a_q_yzero_pt);

std::tuple<int64_t, int32_t, int64_t, int32_t, int32_t, int32_t, int32_t>
qdq_act_matmul_uint16_uint16_cstm(float a_dq_xscale, int64_t a_dq_xzero_pt,
                                  int64_t in_ch_dim, float w_dq_xscale,
                                  int64_t w_dq_xzero_pt, float a_q_yscale,
                                  int64_t a_q_yzero_pt);

std::vector<int32_t> mha_qdq_params_fill(
    std::tuple<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>
        qkt_qdq,
    std::tuple<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>
        smv_qdq,
    std::tuple<int64_t, int64_t> sm_qdq_before,
    std::tuple<int64_t, int64_t> sm_qdq_after, int64_t is_qkt_smv_int16);

std::vector<int32_t> mha_channel_qdq_params_fill(
    std::tuple<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>
        qkt_qdq,
    std::tuple<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>
        smv_qdq,
    std::tuple<int64_t, int64_t> sm_qdq_before,
    std::tuple<int64_t, int64_t> sm_qdq_after,
    std::tuple<int64_t, int64_t> qdq_mul_in,
    std::tuple<int64_t, int64_t> qdq_mul_out, int64_t is_qkt_smv_int16,
    int64_t smv_swap = 0);

std::vector<int32_t> DeMHA_qdq_params_fill(
    std::tuple<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>
        qkt_qdq,
    std::tuple<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>
        smv_qdq,
    std::tuple<int64_t, int64_t> sm_qdq_before,
    std::tuple<int64_t, int64_t> sm_qdq_after,
    std::tuple<float, int16_t, float, int16_t, float, int16_t>
        qdq_pos_con_add_in,
    std::tuple<float, int16_t, float, int16_t, float, int16_t> qdq_att_mask,
    int64_t is_qkt_smv_int16, int64_t smv_swap = 0);
std::tuple<std::vector<int64_t>, int64_t, int64_t, int64_t, int64_t>
dq_uint16A_uint8W_conv_q_param_gen(float in_s, uint16_t in_zp,
                                   gsl::span<const uint8_t> w, float w_s,
                                   uint8_t w_zp,
                                   const std::vector<int64_t>& w_shape,
                                   gsl::span<const int32_t> b, float b_s,
                                   int32_t b_zp, float o_s, uint16_t o_zp);
std::tuple<std::vector<int64_t>, int64_t, int64_t, int64_t, int64_t>
dq_uint16A_int8W_conv_q_param_gen(float in_s, uint16_t in_zp,
                                  gsl::span<const int8_t> w, float w_s,
                                  int8_t w_zp,
                                  const std::vector<int64_t>& w_shape,
                                  gsl::span<const int32_t> b, float b_s,
                                  int32_t b_zp, float o_s, uint16_t o_zp);
std::tuple<std::vector<int64_t>, std::vector<int32_t>, std::vector<int32_t>,
           int64_t, int64_t>
dq_uint16A_int4W_conv_chwise_q_param_gen(
    float in_s, uint16_t in_zp, const std::vector<int8_t>& w,
    gsl::span<const float> w_s, const std::vector<int8_t>& w_zp,
    const std::vector<int64_t>& w_shape, gsl::span<const int32_t> b, float b_s,
    int32_t b_zp, float o_s, uint16_t o_zp);
std::tuple<std::vector<int64_t>, std::vector<int32_t>, std::vector<int32_t>,
           int64_t, int64_t>
dq_uint16A_int4W_conv_chwise_bias_add_q_param_gen(
    float in_s, uint16_t in_zp, const std::vector<int8_t>& w,
    gsl::span<const float> w_s, const std::vector<int8_t>& w_zp,
    const std::vector<int64_t>& w_shape, gsl::span<const uint16_t> b, float b_s,
    uint16_t b_zp, float o_s, uint16_t o_zp);
std::tuple<std::vector<int64_t>, int64_t, int64_t, int64_t, int64_t>
dq_uint16A_uint8W_conv_q_param_gen_shiftmax(float in_s, uint16_t in_zp,
                                            gsl::span<const uint8_t> w,
                                            float w_s, uint8_t w_zp,
                                            const std::vector<int64_t>& w_shape,
                                            gsl::span<const int32_t> b,
                                            float b_s, int32_t b_zp, float o_s,
                                            uint16_t o_zp);
std::tuple<std::vector<int64_t>, int64_t, int64_t, int64_t, int64_t>
dq_uint16A_uint16W_conv_q_param_gen(float in_s, uint16_t in_zp,
                                    gsl::span<const uint16_t> w, float w_s,
                                    uint16_t w_zp,
                                    const std::vector<int64_t>& w_shape,
                                    gsl::span<const int32_t> b, float b_s,
                                    int32_t b_zp, float o_s, uint16_t o_zp);

std::tuple<std::vector<int64_t>, int32_t, int64_t, int64_t, int64_t, int64_t>
dq_uint16A_uint8W_bias_matmul_q_param_gen(
    float a_dq_xscale, uint16_t a_dq_xzero_pt,
    const std::vector<std::vector<uint8_t>>& weights, float w_dq_xscale,
    uint16_t w_dq_xzero_pt, const std::vector<uint16_t>& bias,
    float b_dq_xscale, uint16_t b_dq_xzero_pt, float a_q_yscale,
    uint16_t a_q_yzero_pt);

std::tuple<std::vector<int64_t>, int32_t, int64_t, int64_t, int64_t, int64_t>
dq_uint16A_uint16W_bias_matmul_q_param_gen(
    float a_dq_xscale, uint16_t a_dq_xzero_pt,
    const std::vector<std::vector<uint16_t>>& weights, float w_dq_xscale,
    uint16_t w_dq_xzero_pt, const std::vector<uint16_t>& bias,
    float b_dq_xscale, uint16_t b_dq_xzero_pt, float a_q_yscale,
    uint16_t a_q_yzero_pt, std::vector<int> shifts);

std::vector<int32_t>
mha_qdq_params_fill(const std::tuple<int64_t, int32_t, int64_t, int32_t,
                                     int32_t, int32_t, int32_t>& coeff_qkt,
                    const std::tuple<int64_t, int32_t, int64_t, int32_t,
                                     int32_t, int32_t, int32_t>& coeff_smv,
                    const std::tuple<uint16_t, int>& sm_qdq_before,
                    const std::tuple<uint16_t, int>& sm_qdq_after,
                    int32_t is_qkt_smv_int16);

std::vector<uint8_t>
mladfelwmul_qdq_param_gen(float ifm1_scale, float ifm2_scale, float ofm_scale,
                          uint16_t ifm1_zp, uint16_t ifm2_zp, uint16_t ofm_zp,
                          int64_t tensor_sz);

} // namespace vaip::dd::qmatmulcalc