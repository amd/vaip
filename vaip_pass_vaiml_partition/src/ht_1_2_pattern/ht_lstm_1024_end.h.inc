//  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
//  Licensed under the MIT License.
//
/** generated by the following command:
env \
 IGNORE_CONSTANT=1 \
 ENABLE_CONSTNAT_SHARING=0 \
 $BUILD/vaip/onnxruntime_vitisai_ep/onnx_pattern_gen \
 -i /decoder/rnn/Squeeze_output_0 \
 -i /decoder/rnn/Slice_27_output_0_QuantizeLinear_Output \
 -i /decoder/rnn/Slice_26_output_0_QuantizeLinear_Output \
 -o hidden_QuantizeLinear_Output \
 -f C:\temp\zhonnian\vaip\.cache\6809517bbd0a7b44acc31942d410ca3a\cloned_graph.onnx\
 -c onnx_grep_cxx_pattern.h.inc
*/
auto input_0 = builder.wildcard(); //  id = 0  node_arg_name = /decoder/rnn/Squeeze_output_0
builder.bind("/decoder/rnn/Squeeze_output_0",input_0);
auto constant_0 = builder.constant(); //  id = 1  node_arg_name = /decoder/rnn/LSTM_output_1_scale
builder.bind("/decoder/rnn/LSTM_output_1_scale",constant_0);
auto constant_1 = builder.constant(); //  id = 2  node_arg_name = /decoder/rnn/LSTM_output_0_zero_point
builder.bind("/decoder/rnn/LSTM_output_0_zero_point",constant_1);
auto QuantizeLinear_0 = builder.node2("QuantizeLinear",{input_0,constant_0,constant_1}); //  id = 3  node_arg_name = /decoder/rnn/Squeeze_output_0_QuantizeLinear_Output
builder.bind("/decoder/rnn/Squeeze_output_0_QuantizeLinear_Output",QuantizeLinear_0);
auto constant_2 = builder.constant(); //  id = 4  node_arg_name = /decoder/rnn/LSTM_output_1_scale
builder.bind("/decoder/rnn/LSTM_output_1_scale",constant_2);
auto constant_3 = builder.constant(); //  id = 5  node_arg_name = /decoder/rnn/LSTM_output_0_zero_point
builder.bind("/decoder/rnn/LSTM_output_0_zero_point",constant_3);
auto DequantizeLinear_0 = builder.node2("DequantizeLinear",{QuantizeLinear_0,constant_2,constant_3}); //  id = 6  node_arg_name = /decoder/rnn/Squeeze_output_0_DequantizeLinear_Output
builder.bind("/decoder/rnn/Squeeze_output_0_DequantizeLinear_Output",DequantizeLinear_0);
auto input_1 = builder.wildcard(); //  id = 7  node_arg_name = /decoder/rnn/Slice_26_output_0_QuantizeLinear_Output
builder.bind("/decoder/rnn/Slice_26_output_0_QuantizeLinear_Output",input_1);
auto constant_4 = builder.constant(); //  id = 8  node_arg_name = /decoder/rnn/Slice_26_output_0_scale
builder.bind("/decoder/rnn/Slice_26_output_0_scale",constant_4);
auto constant_5 = builder.constant(); //  id = 9  node_arg_name = /decoder/rnn/Slice_26_output_0_zero_point
builder.bind("/decoder/rnn/Slice_26_output_0_zero_point",constant_5);
auto DequantizeLinear_1 = builder.node2("DequantizeLinear",{input_1,constant_4,constant_5}); //  id = 10  node_arg_name = /decoder/rnn/Slice_26_output_0_DequantizeLinear_Output
builder.bind("/decoder/rnn/Slice_26_output_0_DequantizeLinear_Output",DequantizeLinear_1);
auto input_2 = builder.wildcard(); //  id = 11  node_arg_name = /decoder/rnn/Slice_27_output_0_QuantizeLinear_Output
builder.bind("/decoder/rnn/Slice_27_output_0_QuantizeLinear_Output",input_2);
auto constant_6 = builder.constant(); //  id = 12  node_arg_name = /decoder/rnn/Slice_27_output_0_scale
builder.bind("/decoder/rnn/Slice_27_output_0_scale",constant_6);
auto constant_7 = builder.constant(); //  id = 13  node_arg_name = /decoder/rnn/Slice_27_output_0_zero_point
builder.bind("/decoder/rnn/Slice_27_output_0_zero_point",constant_7);
auto DequantizeLinear_2 = builder.node2("DequantizeLinear",{input_2,constant_6,constant_7}); //  id = 14  node_arg_name = /decoder/rnn/Slice_27_output_0_DequantizeLinear_Output
builder.bind("/decoder/rnn/Slice_27_output_0_DequantizeLinear_Output",DequantizeLinear_2);
auto constant_8 = builder.constant(); //  id = 15  node_arg_name = /decoder/rnn/Unsqueeze_5_output_0_quantized
builder.bind("/decoder/rnn/Unsqueeze_5_output_0_quantized",constant_8);
auto constant_9 = builder.constant(); //  id = 16  node_arg_name = /decoder/rnn/Unsqueeze_5_output_0_scale
builder.bind("/decoder/rnn/Unsqueeze_5_output_0_scale",constant_9);
auto constant_10 = builder.constant(); //  id = 17  node_arg_name = /decoder/rnn/Unsqueeze_5_output_0_zero_point
builder.bind("/decoder/rnn/Unsqueeze_5_output_0_zero_point",constant_10);
auto DequantizeLinear_3 = builder.node2("DequantizeLinear",{constant_8,constant_9,constant_10}); //  id = 18  node_arg_name = /decoder/rnn/Unsqueeze_5_output_0_DequantizeLinear_Output
builder.bind("/decoder/rnn/Unsqueeze_5_output_0_DequantizeLinear_Output",DequantizeLinear_3);
auto constant_11 = builder.constant(); //  id = 19  node_arg_name = /decoder/rnn/Unsqueeze_4_output_0_quantized
builder.bind("/decoder/rnn/Unsqueeze_4_output_0_quantized",constant_11);
auto constant_12 = builder.constant(); //  id = 20  node_arg_name = /decoder/rnn/Unsqueeze_4_output_0_scale
builder.bind("/decoder/rnn/Unsqueeze_4_output_0_scale",constant_12);
auto constant_13 = builder.constant(); //  id = 21  node_arg_name = /decoder/rnn/Unsqueeze_4_output_0_zero_point
builder.bind("/decoder/rnn/Unsqueeze_4_output_0_zero_point",constant_13);
auto DequantizeLinear_4 = builder.node2("DequantizeLinear",{constant_11,constant_12,constant_13}); //  id = 22  node_arg_name = /decoder/rnn/Unsqueeze_4_output_0_DequantizeLinear_Output
builder.bind("/decoder/rnn/Unsqueeze_4_output_0_DequantizeLinear_Output",DequantizeLinear_4);
auto constant_14 = builder.constant(); //  id = 23  node_arg_name = /decoder/rnn/Unsqueeze_3_output_0_quantized
builder.bind("/decoder/rnn/Unsqueeze_3_output_0_quantized",constant_14);
auto constant_15 = builder.constant(); //  id = 24  node_arg_name = /decoder/rnn/Unsqueeze_3_output_0_scale
builder.bind("/decoder/rnn/Unsqueeze_3_output_0_scale",constant_15);
auto constant_16 = builder.constant(); //  id = 25  node_arg_name = /decoder/rnn/Unsqueeze_3_output_0_zero_point
builder.bind("/decoder/rnn/Unsqueeze_3_output_0_zero_point",constant_16);
auto DequantizeLinear_5 = builder.node2("DequantizeLinear",{constant_14,constant_15,constant_16}); //  id = 26  node_arg_name = /decoder/rnn/Unsqueeze_3_output_0_DequantizeLinear_Output
builder.bind("/decoder/rnn/Unsqueeze_3_output_0_DequantizeLinear_Output",DequantizeLinear_5);
auto input_3 = builder.wildcard(); //  id = 27  node_arg_name = 
builder.bind("",input_3);
auto LSTM_0 = builder.node2("LSTM",{DequantizeLinear_0,DequantizeLinear_5,DequantizeLinear_4,DequantizeLinear_3,input_3,DequantizeLinear_1,DequantizeLinear_2}); //  id = 28  node_arg_name = /decoder/rnn/LSTM_1_output_0
builder.bind("/decoder/rnn/LSTM_1_output_0",LSTM_0);
auto constant_17 = builder.constant(); //  id = 29  node_arg_name = /decoder/rnn/LSTM_1_output_0_scale
builder.bind("/decoder/rnn/LSTM_1_output_0_scale",constant_17);
auto constant_18 = builder.constant(); //  id = 30  node_arg_name = /decoder/rnn/LSTM_1_output_0_zero_point
builder.bind("/decoder/rnn/LSTM_1_output_0_zero_point",constant_18);
auto QuantizeLinear_1 = builder.node2("QuantizeLinear",{LSTM_0,constant_17,constant_18}); //  id = 31  node_arg_name = /decoder/rnn/LSTM_1_output_0_QuantizeLinear_Output
builder.bind("/decoder/rnn/LSTM_1_output_0_QuantizeLinear_Output",QuantizeLinear_1);
auto constant_19 = builder.constant(); //  id = 32  node_arg_name = /decoder/rnn/LSTM_1_output_0_scale
builder.bind("/decoder/rnn/LSTM_1_output_0_scale",constant_19);
auto constant_20 = builder.constant(); //  id = 33  node_arg_name = /decoder/rnn/LSTM_1_output_0_zero_point
builder.bind("/decoder/rnn/LSTM_1_output_0_zero_point",constant_20);
auto DequantizeLinear_6 = builder.node2("DequantizeLinear",{QuantizeLinear_1,constant_19,constant_20}); //  id = 34  node_arg_name = /decoder/rnn/LSTM_1_output_0_DequantizeLinear_Output
builder.bind("/decoder/rnn/LSTM_1_output_0_DequantizeLinear_Output",DequantizeLinear_6);
auto constant_21 = builder.constant(); //  id = 35  node_arg_name = /decoder/rnn/Constant_38_output_0
builder.bind("/decoder/rnn/Constant_38_output_0",constant_21);
auto Squeeze_0 = builder.node2("Squeeze",{DequantizeLinear_6,constant_21}); //  id = 36  node_arg_name = /decoder/rnn/Squeeze_1_output_0
builder.bind("/decoder/rnn/Squeeze_1_output_0",Squeeze_0);
auto constant_22 = builder.constant(); //  id = 37  node_arg_name = /decoder/rnn/LSTM_1_output_0_scale
builder.bind("/decoder/rnn/LSTM_1_output_0_scale",constant_22);
auto constant_23 = builder.constant(); //  id = 38  node_arg_name = /decoder/rnn/LSTM_1_output_0_zero_point
builder.bind("/decoder/rnn/LSTM_1_output_0_zero_point",constant_23);
auto QuantizeLinear_2 = builder.node2("QuantizeLinear",{Squeeze_0,constant_22,constant_23}); //  id = 39  node_arg_name = /decoder/rnn/Squeeze_1_output_0_QuantizeLinear_Output
builder.bind("/decoder/rnn/Squeeze_1_output_0_QuantizeLinear_Output",QuantizeLinear_2);
auto constant_24 = builder.constant(); //  id = 40  node_arg_name = /decoder/rnn/LSTM_1_output_0_scale
builder.bind("/decoder/rnn/LSTM_1_output_0_scale",constant_24);
auto constant_25 = builder.constant(); //  id = 41  node_arg_name = /decoder/rnn/LSTM_1_output_0_zero_point
builder.bind("/decoder/rnn/LSTM_1_output_0_zero_point",constant_25);
auto DequantizeLinear_7 = builder.node2("DequantizeLinear",{QuantizeLinear_2,constant_24,constant_25}); //  id = 42  node_arg_name = /decoder/rnn/Squeeze_1_output_0_DequantizeLinear_Output
builder.bind("/decoder/rnn/Squeeze_1_output_0_DequantizeLinear_Output",DequantizeLinear_7);
auto Transpose_0 = builder.node2("Transpose",{DequantizeLinear_7}); //  id = 43  node_arg_name = /decoder/rnn/Transpose_1_output_0
builder.bind("/decoder/rnn/Transpose_1_output_0",Transpose_0);
auto constant_26 = builder.constant(); //  id = 44  node_arg_name = /decoder/rnn/LSTM_1_output_0_scale
builder.bind("/decoder/rnn/LSTM_1_output_0_scale",constant_26);
auto constant_27 = builder.constant(); //  id = 45  node_arg_name = /decoder/rnn/LSTM_1_output_0_zero_point
builder.bind("/decoder/rnn/LSTM_1_output_0_zero_point",constant_27);
auto QuantizeLinear_3 = builder.node2("QuantizeLinear",{Transpose_0,constant_26,constant_27}); //  id = 46  node_arg_name = /decoder/rnn/Transpose_1_output_0_QuantizeLinear_Output
builder.bind("/decoder/rnn/Transpose_1_output_0_QuantizeLinear_Output",QuantizeLinear_3);
auto constant_28 = builder.constant(); //  id = 47  node_arg_name = /decoder/rnn/LSTM_1_output_0_scale
builder.bind("/decoder/rnn/LSTM_1_output_0_scale",constant_28);
auto constant_29 = builder.constant(); //  id = 48  node_arg_name = /decoder/rnn/LSTM_1_output_0_zero_point
builder.bind("/decoder/rnn/LSTM_1_output_0_zero_point",constant_29);
auto DequantizeLinear_8 = builder.node2("DequantizeLinear",{QuantizeLinear_3,constant_28,constant_29}); //  id = 49  node_arg_name = /decoder/rnn/Transpose_1_output_0_DequantizeLinear_Output
builder.bind("/decoder/rnn/Transpose_1_output_0_DequantizeLinear_Output",DequantizeLinear_8);
auto constant_30 = builder.constant(); //  id = 50  node_arg_name = decoder.lnorm_layer.weight_quantized
builder.bind("decoder.lnorm_layer.weight_quantized",constant_30);
auto constant_31 = builder.constant(); //  id = 51  node_arg_name = decoder.lnorm_layer.weight_scale
builder.bind("decoder.lnorm_layer.weight_scale",constant_31);
auto constant_32 = builder.constant(); //  id = 52  node_arg_name = decoder.lnorm_layer.weight_zero_point
builder.bind("decoder.lnorm_layer.weight_zero_point",constant_32);
auto DequantizeLinear_9 = builder.node2("DequantizeLinear",{constant_30,constant_31,constant_32}); //  id = 53  node_arg_name = decoder.lnorm_layer.weight_DequantizeLinear_Output
builder.bind("decoder.lnorm_layer.weight_DequantizeLinear_Output",DequantizeLinear_9);
auto constant_33 = builder.constant(); //  id = 54  node_arg_name = decoder.lnorm_layer.bias_quantized
builder.bind("decoder.lnorm_layer.bias_quantized",constant_33);
auto constant_34 = builder.constant(); //  id = 55  node_arg_name = decoder.lnorm_layer.bias_quantized_scale
builder.bind("decoder.lnorm_layer.bias_quantized_scale",constant_34);
auto constant_35 = builder.constant(); //  id = 56  node_arg_name = decoder.lnorm_layer.bias_quantized_zero_point
builder.bind("decoder.lnorm_layer.bias_quantized_zero_point",constant_35);
auto DequantizeLinear_10 = builder.node2("DequantizeLinear",{constant_33,constant_34,constant_35}); //  id = 57  node_arg_name = decoder.lnorm_layer.bias
builder.bind("decoder.lnorm_layer.bias",DequantizeLinear_10);
auto LayerNormalization_0 = builder.node2("LayerNormalization",{DequantizeLinear_8,DequantizeLinear_9,DequantizeLinear_10}); //  id = 58  node_arg_name = /decoder/lnorm_layer/Add_1_output_0
builder.bind("/decoder/lnorm_layer/Add_1_output_0",LayerNormalization_0);
auto constant_36 = builder.constant(); //  id = 59  node_arg_name = /decoder/lnorm_layer/Add_1_output_0_scale
builder.bind("/decoder/lnorm_layer/Add_1_output_0_scale",constant_36);
auto constant_37 = builder.constant(); //  id = 60  node_arg_name = /decoder/lnorm_layer/Add_1_output_0_zero_point
builder.bind("/decoder/lnorm_layer/Add_1_output_0_zero_point",constant_37);
auto QuantizeLinear_4 = builder.node2("QuantizeLinear",{LayerNormalization_0,constant_36,constant_37}); //  id = 61  node_arg_name = /decoder/lnorm_layer/Add_1_output_0_QuantizeLinear_Output
builder.bind("/decoder/lnorm_layer/Add_1_output_0_QuantizeLinear_Output",QuantizeLinear_4);
auto constant_38 = builder.constant(); //  id = 62  node_arg_name = /decoder/lnorm_layer/Add_1_output_0_scale
builder.bind("/decoder/lnorm_layer/Add_1_output_0_scale",constant_38);
auto constant_39 = builder.constant(); //  id = 63  node_arg_name = /decoder/lnorm_layer/Add_1_output_0_zero_point
builder.bind("/decoder/lnorm_layer/Add_1_output_0_zero_point",constant_39);
auto DequantizeLinear_11 = builder.node2("DequantizeLinear",{QuantizeLinear_4,constant_38,constant_39}); //  id = 64  node_arg_name = /decoder/lnorm_layer/Add_1_output_0_DequantizeLinear_Output
builder.bind("/decoder/lnorm_layer/Add_1_output_0_DequantizeLinear_Output",DequantizeLinear_11);
auto constant_40 = builder.constant(); //  id = 65  node_arg_name = /lin_dec/fc/Transpose_output_0_quantized
builder.bind("/lin_dec/fc/Transpose_output_0_quantized",constant_40);
auto constant_41 = builder.constant(); //  id = 66  node_arg_name = /lin_dec/fc/Transpose_output_0_scale
builder.bind("/lin_dec/fc/Transpose_output_0_scale",constant_41);
auto constant_42 = builder.constant(); //  id = 67  node_arg_name = /lin_dec/fc/Transpose_output_0_zero_point
builder.bind("/lin_dec/fc/Transpose_output_0_zero_point",constant_42);
auto DequantizeLinear_12 = builder.node2("DequantizeLinear",{constant_40,constant_41,constant_42}); //  id = 68  node_arg_name = /lin_dec/fc/Transpose_output_0_DequantizeLinear_Output
builder.bind("/lin_dec/fc/Transpose_output_0_DequantizeLinear_Output",DequantizeLinear_12);
auto MatMul_0 = builder.node2("MatMul",{DequantizeLinear_11,DequantizeLinear_12}); //  id = 69  node_arg_name = /lin_dec/fc/MatMul_output_0
builder.bind("/lin_dec/fc/MatMul_output_0",MatMul_0);
auto constant_43 = builder.constant(); //  id = 70  node_arg_name = /lin_dec/fc/MatMul_output_0_scale
builder.bind("/lin_dec/fc/MatMul_output_0_scale",constant_43);
auto constant_44 = builder.constant(); //  id = 71  node_arg_name = /lin_dec/fc/MatMul_output_0_zero_point
builder.bind("/lin_dec/fc/MatMul_output_0_zero_point",constant_44);
auto QuantizeLinear_5 = builder.node2("QuantizeLinear",{MatMul_0,constant_43,constant_44}); //  id = 72  node_arg_name = /lin_dec/fc/MatMul_output_0_QuantizeLinear_Output
builder.bind("/lin_dec/fc/MatMul_output_0_QuantizeLinear_Output",QuantizeLinear_5);
auto constant_45 = builder.constant(); //  id = 73  node_arg_name = /lin_dec/fc/MatMul_output_0_scale
builder.bind("/lin_dec/fc/MatMul_output_0_scale",constant_45);
auto constant_46 = builder.constant(); //  id = 74  node_arg_name = /lin_dec/fc/MatMul_output_0_zero_point
builder.bind("/lin_dec/fc/MatMul_output_0_zero_point",constant_46);
auto DequantizeLinear_13 = builder.node2("DequantizeLinear",{QuantizeLinear_5,constant_45,constant_46}); //  id = 75  node_arg_name = /lin_dec/fc/MatMul_output_0_DequantizeLinear_Output
builder.bind("/lin_dec/fc/MatMul_output_0_DequantizeLinear_Output",DequantizeLinear_13);
auto constant_47 = builder.constant(); //  id = 76  node_arg_name = joint_network.lin_dec.fc.bias_quantized
builder.bind("joint_network.lin_dec.fc.bias_quantized",constant_47);
auto constant_48 = builder.constant(); //  id = 77  node_arg_name = joint_network.lin_dec.fc.bias_scale
builder.bind("joint_network.lin_dec.fc.bias_scale",constant_48);
auto constant_49 = builder.constant(); //  id = 78  node_arg_name = joint_network.lin_dec.fc.bias_zero_point
builder.bind("joint_network.lin_dec.fc.bias_zero_point",constant_49);
auto DequantizeLinear_14 = builder.node2("DequantizeLinear",{constant_47,constant_48,constant_49}); //  id = 79  node_arg_name = joint_network.lin_dec.fc.bias_DequantizeLinear_Output
builder.bind("joint_network.lin_dec.fc.bias_DequantizeLinear_Output",DequantizeLinear_14);
auto Add_0 = builder.node2("Add",{DequantizeLinear_14,DequantizeLinear_13}); //  id = 80  node_arg_name = /lin_dec/fc/Add_output_0
builder.bind("/lin_dec/fc/Add_output_0",Add_0);
auto constant_50 = builder.constant(); //  id = 81  node_arg_name = /lin_dec/fc/Add_output_0_scale
builder.bind("/lin_dec/fc/Add_output_0_scale",constant_50);
auto constant_51 = builder.constant(); //  id = 82  node_arg_name = /lin_dec/fc/Add_output_0_zero_point
builder.bind("/lin_dec/fc/Add_output_0_zero_point",constant_51);
auto QuantizeLinear_6 = builder.node2("QuantizeLinear",{Add_0,constant_50,constant_51}); //  id = 83  node_arg_name = /lin_dec/fc/Add_output_0_QuantizeLinear_Output
builder.bind("/lin_dec/fc/Add_output_0_QuantizeLinear_Output",QuantizeLinear_6);
auto constant_52 = builder.constant(); //  id = 84  node_arg_name = /lin_dec/fc/Add_output_0_scale
builder.bind("/lin_dec/fc/Add_output_0_scale",constant_52);
auto constant_53 = builder.constant(); //  id = 85  node_arg_name = /lin_dec/fc/Add_output_0_zero_point
builder.bind("/lin_dec/fc/Add_output_0_zero_point",constant_53);
auto DequantizeLinear_15 = builder.node2("DequantizeLinear",{QuantizeLinear_6,constant_52,constant_53}); //  id = 86  node_arg_name = /lin_dec/fc/Add_output_0_DequantizeLinear_Output
builder.bind("/lin_dec/fc/Add_output_0_DequantizeLinear_Output",DequantizeLinear_15);
auto constant_54 = builder.constant(); //  id = 87  node_arg_name = joint_network.lin_dec.Lnorm.weight_quantized
builder.bind("joint_network.lin_dec.Lnorm.weight_quantized",constant_54);
auto constant_55 = builder.constant(); //  id = 88  node_arg_name = joint_network.lin_dec.Lnorm.weight_scale
builder.bind("joint_network.lin_dec.Lnorm.weight_scale",constant_55);
auto constant_56 = builder.constant(); //  id = 89  node_arg_name = joint_network.lin_dec.Lnorm.weight_zero_point
builder.bind("joint_network.lin_dec.Lnorm.weight_zero_point",constant_56);
auto DequantizeLinear_16 = builder.node2("DequantizeLinear",{constant_54,constant_55,constant_56}); //  id = 90  node_arg_name = joint_network.lin_dec.Lnorm.weight_DequantizeLinear_Output
builder.bind("joint_network.lin_dec.Lnorm.weight_DequantizeLinear_Output",DequantizeLinear_16);
auto constant_57 = builder.constant(); //  id = 91  node_arg_name = joint_network.lin_dec.Lnorm.bias_quantized
builder.bind("joint_network.lin_dec.Lnorm.bias_quantized",constant_57);
auto constant_58 = builder.constant(); //  id = 92  node_arg_name = joint_network.lin_dec.Lnorm.bias_quantized_scale
builder.bind("joint_network.lin_dec.Lnorm.bias_quantized_scale",constant_58);
auto constant_59 = builder.constant(); //  id = 93  node_arg_name = decoder.lnorm_layer.bias_quantized_zero_point
builder.bind("decoder.lnorm_layer.bias_quantized_zero_point",constant_59);
auto DequantizeLinear_17 = builder.node2("DequantizeLinear",{constant_57,constant_58,constant_59}); //  id = 94  node_arg_name = joint_network.lin_dec.Lnorm.bias
builder.bind("joint_network.lin_dec.Lnorm.bias",DequantizeLinear_17);
auto LayerNormalization_1 = builder.node2("LayerNormalization",{DequantizeLinear_15,DequantizeLinear_16,DequantizeLinear_17}); //  id = 95  node_arg_name = hidden_QuantizeLinear_Input
builder.bind("hidden_QuantizeLinear_Input",LayerNormalization_1);
auto constant_60 = builder.constant(); //  id = 96  node_arg_name = hidden_scale
builder.bind("hidden_scale",constant_60);
auto constant_61 = builder.constant(); //  id = 97  node_arg_name = hidden_zero_point
builder.bind("hidden_zero_point",constant_61);
auto QuantizeLinear_7 = builder.node2("QuantizeLinear",{LayerNormalization_1,constant_60,constant_61}); //  id = 98  node_arg_name = hidden_QuantizeLinear_Output
builder.bind("hidden_QuantizeLinear_Output",QuantizeLinear_7);
ret = QuantizeLinear_7;
