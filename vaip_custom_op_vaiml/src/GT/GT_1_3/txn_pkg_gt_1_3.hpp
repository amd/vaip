#pragma once
#include <string>
#include <tuple>
const std::string& getBins_front_sub_ml_txn_1_3(std::string& model_version);
const std::string&
getBins_linear_qkv_ln_bmm_concat_ctrl_pkt_1_3(std::string& model_version);
const std::string&
getBins_linear_qkv_ln_bmm_concat_ml_txn_1_3(std::string& model_version);
const std::string&
getBins_ln_matmul_bias_ln_ctrl_pkt_1_3(std::string& model_version);
const std::string&
getBins_ln_matmul_bias_ln_ml_txn_1_3(std::string& model_version);
const std::string&
getBins_matmul_reduce_ctrl_pkt_1_3(std::string& model_version);
const std::string& getBins_matmul_reduce_ml_txn_1_3(std::string& model_version);
const std::string&
getBins_out_matmul_bias_ctrl_pkt_1_3(std::string& model_version);
const std::string&
getBins_out_matmul_bias_ml_txn_1_3(std::string& model_version);
const std::string& getBins_softmax_linear_out_feed_forward_ctrl_pkt_1_3(
    std::string& model_version);
const std::string&
getBins_softmax_linear_out_feed_forward_ml_txn_1_3(std::string& model_version);
const std::string&
getBins_transformer_layers_ctrl_pkt_1_3(std::string& model_version);
const std::string&
getBins_transformer_layers_ml_txn_1_3(std::string& model_version);