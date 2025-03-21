//
//  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
//  Licensed under the MIT License.
//

/** generated by the following command:
env \
 IGNORE_CONSTANT=1 \
 ENABLE_CONSTNAT_SHARING=0 \
 $BUILD/vaip/onnxruntime_vitisai_ep/onnx_pattern_gen \
 -i /Slice_102_output_0_QuantizeLinear_Output \
 -i /Slice_106_output_0_QuantizeLinear_Output \
 -i /Slice_10_output_0_QuantizeLinear_Output \
 -i /Slice_110_output_0_QuantizeLinear_Output \
 -i /Slice_114_output_0_QuantizeLinear_Output \
 -i /Slice_118_output_0_QuantizeLinear_Output \
 -i /Slice_122_output_0_QuantizeLinear_Output \
 -i /Slice_126_output_0_QuantizeLinear_Output \
 -i /Slice_14_output_0_QuantizeLinear_Output \
 -i /Slice_18_output_0_QuantizeLinear_Output \
 -i /Slice_22_output_0_QuantizeLinear_Output \
 -i /Slice_26_output_0_QuantizeLinear_Output \
 -i /Slice_2_output_0_QuantizeLinear_Output \
 -i /Slice_30_output_0_QuantizeLinear_Output \
 -i /Slice_34_output_0_QuantizeLinear_Output \
 -i /Slice_38_output_0_QuantizeLinear_Output \
 -i /Slice_42_output_0_QuantizeLinear_Output \
 -i /Slice_46_output_0_QuantizeLinear_Output \
 -i /Slice_50_output_0_QuantizeLinear_Output \
 -i /Slice_54_output_0_QuantizeLinear_Output \
 -i /Slice_58_output_0_QuantizeLinear_Output \
 -i /Slice_62_output_0_QuantizeLinear_Output \
 -i /Slice_66_output_0_QuantizeLinear_Output \
 -i /Slice_6_output_0_QuantizeLinear_Output \
 -i /Slice_70_output_0_QuantizeLinear_Output \
 -i /Slice_74_output_0_QuantizeLinear_Output \
 -i /Slice_78_output_0_QuantizeLinear_Output \
 -i /Slice_82_output_0_QuantizeLinear_Output \
 -i /Slice_86_output_0_QuantizeLinear_Output \
 -i /Slice_90_output_0_QuantizeLinear_Output \
 -i /Slice_94_output_0_QuantizeLinear_Output \
 -i /Slice_98_output_0_QuantizeLinear_Output \
 -o oup_cache_k \
 -f C:\temp\zhonnian\vaip\.cache\426c5dd1cf2fc5d303d4fb1288dca3d7\cloned_graph.onnx\
 -c onnx_grep_cxx_pattern.h.inc
*/
auto input_0 = builder.wildcard(); //  id = 0  node_arg_name = /Slice_126_output_0_QuantizeLinear_Output
builder.bind("/Slice_126_output_0_QuantizeLinear_Output",input_0);
auto constant_0 = builder.constant(); //  id = 1  node_arg_name = /Concat_350_output_0_scale
builder.bind("/Concat_350_output_0_scale",constant_0);
auto constant_1 = builder.constant(); //  id = 2  node_arg_name = /Concat_350_output_0_zero_point
builder.bind("/Concat_350_output_0_zero_point",constant_1);
auto com_microsoft_DequantizeLinear_0 = builder.node2("com.microsoft:DequantizeLinear",{input_0,constant_0,constant_1}); //  id = 3  node_arg_name = /Slice_126_output_0_DequantizeLinear_Output
builder.bind("/Slice_126_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_0);
auto input_1 = builder.wildcard(); //  id = 4  node_arg_name = /Slice_122_output_0_QuantizeLinear_Output
builder.bind("/Slice_122_output_0_QuantizeLinear_Output",input_1);
auto constant_2 = builder.constant(); //  id = 5  node_arg_name = /Concat_342_output_0_scale
builder.bind("/Concat_342_output_0_scale",constant_2);
auto constant_3 = builder.constant(); //  id = 6  node_arg_name = /Concat_342_output_0_zero_point
builder.bind("/Concat_342_output_0_zero_point",constant_3);
auto com_microsoft_DequantizeLinear_1 = builder.node2("com.microsoft:DequantizeLinear",{input_1,constant_2,constant_3}); //  id = 7  node_arg_name = /Slice_122_output_0_DequantizeLinear_Output
builder.bind("/Slice_122_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_1);
auto input_2 = builder.wildcard(); //  id = 8  node_arg_name = /Slice_118_output_0_QuantizeLinear_Output
builder.bind("/Slice_118_output_0_QuantizeLinear_Output",input_2);
auto constant_4 = builder.constant(); //  id = 9  node_arg_name = /Concat_334_output_0_scale
builder.bind("/Concat_334_output_0_scale",constant_4);
auto constant_5 = builder.constant(); //  id = 10  node_arg_name = /Concat_334_output_0_zero_point
builder.bind("/Concat_334_output_0_zero_point",constant_5);
auto com_microsoft_DequantizeLinear_2 = builder.node2("com.microsoft:DequantizeLinear",{input_2,constant_4,constant_5}); //  id = 11  node_arg_name = /Slice_118_output_0_DequantizeLinear_Output
builder.bind("/Slice_118_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_2);
auto input_3 = builder.wildcard(); //  id = 12  node_arg_name = /Slice_114_output_0_QuantizeLinear_Output
builder.bind("/Slice_114_output_0_QuantizeLinear_Output",input_3);
auto constant_6 = builder.constant(); //  id = 13  node_arg_name = /linear_k_28/Add_output_0_scale
builder.bind("/linear_k_28/Add_output_0_scale",constant_6);
auto constant_7 = builder.constant(); //  id = 14  node_arg_name = /Concat_326_output_0_zero_point
builder.bind("/Concat_326_output_0_zero_point",constant_7);
auto com_microsoft_DequantizeLinear_3 = builder.node2("com.microsoft:DequantizeLinear",{input_3,constant_6,constant_7}); //  id = 15  node_arg_name = /Slice_114_output_0_DequantizeLinear_Output
builder.bind("/Slice_114_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_3);
auto input_4 = builder.wildcard(); //  id = 16  node_arg_name = /Slice_110_output_0_QuantizeLinear_Output
builder.bind("/Slice_110_output_0_QuantizeLinear_Output",input_4);
auto constant_8 = builder.constant(); //  id = 17  node_arg_name = /Unsqueeze_1237_output_0_scale
builder.bind("/Unsqueeze_1237_output_0_scale",constant_8);
auto constant_9 = builder.constant(); //  id = 18  node_arg_name = /Concat_318_output_0_zero_point
builder.bind("/Concat_318_output_0_zero_point",constant_9);
auto com_microsoft_DequantizeLinear_4 = builder.node2("com.microsoft:DequantizeLinear",{input_4,constant_8,constant_9}); //  id = 19  node_arg_name = /Slice_110_output_0_DequantizeLinear_Output
builder.bind("/Slice_110_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_4);
auto input_5 = builder.wildcard(); //  id = 20  node_arg_name = /Slice_106_output_0_QuantizeLinear_Output
builder.bind("/Slice_106_output_0_QuantizeLinear_Output",input_5);
auto constant_10 = builder.constant(); //  id = 21  node_arg_name = /Concat_310_output_0_scale
builder.bind("/Concat_310_output_0_scale",constant_10);
auto constant_11 = builder.constant(); //  id = 22  node_arg_name = /Concat_310_output_0_zero_point
builder.bind("/Concat_310_output_0_zero_point",constant_11);
auto com_microsoft_DequantizeLinear_5 = builder.node2("com.microsoft:DequantizeLinear",{input_5,constant_10,constant_11}); //  id = 23  node_arg_name = /Slice_106_output_0_DequantizeLinear_Output
builder.bind("/Slice_106_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_5);
auto input_6 = builder.wildcard(); //  id = 24  node_arg_name = /Slice_102_output_0_QuantizeLinear_Output
builder.bind("/Slice_102_output_0_QuantizeLinear_Output",input_6);
auto constant_12 = builder.constant(); //  id = 25  node_arg_name = /linear_k_25/Add_output_0_scale
builder.bind("/linear_k_25/Add_output_0_scale",constant_12);
auto constant_13 = builder.constant(); //  id = 26  node_arg_name = /Concat_302_output_0_zero_point
builder.bind("/Concat_302_output_0_zero_point",constant_13);
auto com_microsoft_DequantizeLinear_6 = builder.node2("com.microsoft:DequantizeLinear",{input_6,constant_12,constant_13}); //  id = 27  node_arg_name = /Slice_102_output_0_DequantizeLinear_Output
builder.bind("/Slice_102_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_6);
auto input_7 = builder.wildcard(); //  id = 28  node_arg_name = /Slice_98_output_0_QuantizeLinear_Output
builder.bind("/Slice_98_output_0_QuantizeLinear_Output",input_7);
auto constant_14 = builder.constant(); //  id = 29  node_arg_name = /Concat_294_output_0_scale
builder.bind("/Concat_294_output_0_scale",constant_14);
auto constant_15 = builder.constant(); //  id = 30  node_arg_name = /Concat_294_output_0_zero_point
builder.bind("/Concat_294_output_0_zero_point",constant_15);
auto com_microsoft_DequantizeLinear_7 = builder.node2("com.microsoft:DequantizeLinear",{input_7,constant_14,constant_15}); //  id = 31  node_arg_name = /Slice_98_output_0_DequantizeLinear_Output
builder.bind("/Slice_98_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_7);
auto input_8 = builder.wildcard(); //  id = 32  node_arg_name = /Slice_94_output_0_QuantizeLinear_Output
builder.bind("/Slice_94_output_0_QuantizeLinear_Output",input_8);
auto constant_16 = builder.constant(); //  id = 33  node_arg_name = /Concat_286_output_0_scale
builder.bind("/Concat_286_output_0_scale",constant_16);
auto constant_17 = builder.constant(); //  id = 34  node_arg_name = /Concat_286_output_0_zero_point
builder.bind("/Concat_286_output_0_zero_point",constant_17);
auto com_microsoft_DequantizeLinear_8 = builder.node2("com.microsoft:DequantizeLinear",{input_8,constant_16,constant_17}); //  id = 35  node_arg_name = /Slice_94_output_0_DequantizeLinear_Output
builder.bind("/Slice_94_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_8);
auto input_9 = builder.wildcard(); //  id = 36  node_arg_name = /Slice_90_output_0_QuantizeLinear_Output
builder.bind("/Slice_90_output_0_QuantizeLinear_Output",input_9);
auto constant_18 = builder.constant(); //  id = 37  node_arg_name = /linear_k_22/Add_output_0_scale
builder.bind("/linear_k_22/Add_output_0_scale",constant_18);
auto constant_19 = builder.constant(); //  id = 38  node_arg_name = /Concat_278_output_0_zero_point
builder.bind("/Concat_278_output_0_zero_point",constant_19);
auto com_microsoft_DequantizeLinear_9 = builder.node2("com.microsoft:DequantizeLinear",{input_9,constant_18,constant_19}); //  id = 39  node_arg_name = /Slice_90_output_0_DequantizeLinear_Output
builder.bind("/Slice_90_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_9);
auto input_10 = builder.wildcard(); //  id = 40  node_arg_name = /Slice_86_output_0_QuantizeLinear_Output
builder.bind("/Slice_86_output_0_QuantizeLinear_Output",input_10);
auto constant_20 = builder.constant(); //  id = 41  node_arg_name = /linear_k_21/Add_output_0_scale
builder.bind("/linear_k_21/Add_output_0_scale",constant_20);
auto constant_21 = builder.constant(); //  id = 42  node_arg_name = /Concat_270_output_0_zero_point
builder.bind("/Concat_270_output_0_zero_point",constant_21);
auto com_microsoft_DequantizeLinear_10 = builder.node2("com.microsoft:DequantizeLinear",{input_10,constant_20,constant_21}); //  id = 43  node_arg_name = /Slice_86_output_0_DequantizeLinear_Output
builder.bind("/Slice_86_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_10);
auto input_11 = builder.wildcard(); //  id = 44  node_arg_name = /Slice_82_output_0_QuantizeLinear_Output
builder.bind("/Slice_82_output_0_QuantizeLinear_Output",input_11);
auto constant_22 = builder.constant(); //  id = 45  node_arg_name = /Unsqueeze_1006_output_0_scale
builder.bind("/Unsqueeze_1006_output_0_scale",constant_22);
auto constant_23 = builder.constant(); //  id = 46  node_arg_name = /Concat_262_output_0_zero_point
builder.bind("/Concat_262_output_0_zero_point",constant_23);
auto com_microsoft_DequantizeLinear_11 = builder.node2("com.microsoft:DequantizeLinear",{input_11,constant_22,constant_23}); //  id = 47  node_arg_name = /Slice_82_output_0_DequantizeLinear_Output
builder.bind("/Slice_82_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_11);
auto input_12 = builder.wildcard(); //  id = 48  node_arg_name = /Slice_78_output_0_QuantizeLinear_Output
builder.bind("/Slice_78_output_0_QuantizeLinear_Output",input_12);
auto constant_24 = builder.constant(); //  id = 49  node_arg_name = /Concat_254_output_0_scale
builder.bind("/Concat_254_output_0_scale",constant_24);
auto constant_25 = builder.constant(); //  id = 50  node_arg_name = /Concat_254_output_0_zero_point
builder.bind("/Concat_254_output_0_zero_point",constant_25);
auto com_microsoft_DequantizeLinear_12 = builder.node2("com.microsoft:DequantizeLinear",{input_12,constant_24,constant_25}); //  id = 51  node_arg_name = /Slice_78_output_0_DequantizeLinear_Output
builder.bind("/Slice_78_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_12);
auto input_13 = builder.wildcard(); //  id = 52  node_arg_name = /Slice_74_output_0_QuantizeLinear_Output
builder.bind("/Slice_74_output_0_QuantizeLinear_Output",input_13);
auto constant_26 = builder.constant(); //  id = 53  node_arg_name = /Concat_246_output_0_scale
builder.bind("/Concat_246_output_0_scale",constant_26);
auto constant_27 = builder.constant(); //  id = 54  node_arg_name = /Concat_246_output_0_zero_point
builder.bind("/Concat_246_output_0_zero_point",constant_27);
auto com_microsoft_DequantizeLinear_13 = builder.node2("com.microsoft:DequantizeLinear",{input_13,constant_26,constant_27}); //  id = 55  node_arg_name = /Slice_74_output_0_DequantizeLinear_Output
builder.bind("/Slice_74_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_13);
auto input_14 = builder.wildcard(); //  id = 56  node_arg_name = /Slice_70_output_0_QuantizeLinear_Output
builder.bind("/Slice_70_output_0_QuantizeLinear_Output",input_14);
auto constant_28 = builder.constant(); //  id = 57  node_arg_name = /Unsqueeze_907_output_0_scale
builder.bind("/Unsqueeze_907_output_0_scale",constant_28);
auto constant_29 = builder.constant(); //  id = 58  node_arg_name = /Concat_238_output_0_zero_point
builder.bind("/Concat_238_output_0_zero_point",constant_29);
auto com_microsoft_DequantizeLinear_14 = builder.node2("com.microsoft:DequantizeLinear",{input_14,constant_28,constant_29}); //  id = 59  node_arg_name = /Slice_70_output_0_DequantizeLinear_Output
builder.bind("/Slice_70_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_14);
auto input_15 = builder.wildcard(); //  id = 60  node_arg_name = /Slice_66_output_0_QuantizeLinear_Output
builder.bind("/Slice_66_output_0_QuantizeLinear_Output",input_15);
auto constant_30 = builder.constant(); //  id = 61  node_arg_name = /Concat_230_output_0_scale
builder.bind("/Concat_230_output_0_scale",constant_30);
auto constant_31 = builder.constant(); //  id = 62  node_arg_name = /Concat_230_output_0_zero_point
builder.bind("/Concat_230_output_0_zero_point",constant_31);
auto com_microsoft_DequantizeLinear_15 = builder.node2("com.microsoft:DequantizeLinear",{input_15,constant_30,constant_31}); //  id = 63  node_arg_name = /Slice_66_output_0_DequantizeLinear_Output
builder.bind("/Slice_66_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_15);
auto input_16 = builder.wildcard(); //  id = 64  node_arg_name = /Slice_62_output_0_QuantizeLinear_Output
builder.bind("/Slice_62_output_0_QuantizeLinear_Output",input_16);
auto constant_32 = builder.constant(); //  id = 65  node_arg_name = /Concat_222_output_0_scale
builder.bind("/Concat_222_output_0_scale",constant_32);
auto constant_33 = builder.constant(); //  id = 66  node_arg_name = /Concat_222_output_0_zero_point
builder.bind("/Concat_222_output_0_zero_point",constant_33);
auto com_microsoft_DequantizeLinear_16 = builder.node2("com.microsoft:DequantizeLinear",{input_16,constant_32,constant_33}); //  id = 67  node_arg_name = /Slice_62_output_0_DequantizeLinear_Output
builder.bind("/Slice_62_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_16);
auto input_17 = builder.wildcard(); //  id = 68  node_arg_name = /Slice_58_output_0_QuantizeLinear_Output
builder.bind("/Slice_58_output_0_QuantizeLinear_Output",input_17);
auto constant_34 = builder.constant(); //  id = 69  node_arg_name = /Concat_214_output_0_scale
builder.bind("/Concat_214_output_0_scale",constant_34);
auto constant_35 = builder.constant(); //  id = 70  node_arg_name = /Concat_214_output_0_zero_point
builder.bind("/Concat_214_output_0_zero_point",constant_35);
auto com_microsoft_DequantizeLinear_17 = builder.node2("com.microsoft:DequantizeLinear",{input_17,constant_34,constant_35}); //  id = 71  node_arg_name = /Slice_58_output_0_DequantizeLinear_Output
builder.bind("/Slice_58_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_17);
auto input_18 = builder.wildcard(); //  id = 72  node_arg_name = /Slice_54_output_0_QuantizeLinear_Output
builder.bind("/Slice_54_output_0_QuantizeLinear_Output",input_18);
auto constant_36 = builder.constant(); //  id = 73  node_arg_name = /Concat_206_output_0_scale
builder.bind("/Concat_206_output_0_scale",constant_36);
auto constant_37 = builder.constant(); //  id = 74  node_arg_name = /Concat_206_output_0_zero_point
builder.bind("/Concat_206_output_0_zero_point",constant_37);
auto com_microsoft_DequantizeLinear_18 = builder.node2("com.microsoft:DequantizeLinear",{input_18,constant_36,constant_37}); //  id = 75  node_arg_name = /Slice_54_output_0_DequantizeLinear_Output
builder.bind("/Slice_54_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_18);
auto input_19 = builder.wildcard(); //  id = 76  node_arg_name = /Slice_50_output_0_QuantizeLinear_Output
builder.bind("/Slice_50_output_0_QuantizeLinear_Output",input_19);
auto constant_38 = builder.constant(); //  id = 77  node_arg_name = /linear_k_12/Add_output_0_scale
builder.bind("/linear_k_12/Add_output_0_scale",constant_38);
auto constant_39 = builder.constant(); //  id = 78  node_arg_name = /Concat_198_output_0_zero_point
builder.bind("/Concat_198_output_0_zero_point",constant_39);
auto com_microsoft_DequantizeLinear_19 = builder.node2("com.microsoft:DequantizeLinear",{input_19,constant_38,constant_39}); //  id = 79  node_arg_name = /Slice_50_output_0_DequantizeLinear_Output
builder.bind("/Slice_50_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_19);
auto input_20 = builder.wildcard(); //  id = 80  node_arg_name = /Slice_46_output_0_QuantizeLinear_Output
builder.bind("/Slice_46_output_0_QuantizeLinear_Output",input_20);
auto constant_40 = builder.constant(); //  id = 81  node_arg_name = /Concat_190_output_0_scale
builder.bind("/Concat_190_output_0_scale",constant_40);
auto constant_41 = builder.constant(); //  id = 82  node_arg_name = /Concat_190_output_0_zero_point
builder.bind("/Concat_190_output_0_zero_point",constant_41);
auto com_microsoft_DequantizeLinear_20 = builder.node2("com.microsoft:DequantizeLinear",{input_20,constant_40,constant_41}); //  id = 83  node_arg_name = /Slice_46_output_0_DequantizeLinear_Output
builder.bind("/Slice_46_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_20);
auto input_21 = builder.wildcard(); //  id = 84  node_arg_name = /Slice_42_output_0_QuantizeLinear_Output
builder.bind("/Slice_42_output_0_QuantizeLinear_Output",input_21);
auto constant_42 = builder.constant(); //  id = 85  node_arg_name = /Concat_182_output_0_scale
builder.bind("/Concat_182_output_0_scale",constant_42);
auto constant_43 = builder.constant(); //  id = 86  node_arg_name = /Concat_182_output_0_zero_point
builder.bind("/Concat_182_output_0_zero_point",constant_43);
auto com_microsoft_DequantizeLinear_21 = builder.node2("com.microsoft:DequantizeLinear",{input_21,constant_42,constant_43}); //  id = 87  node_arg_name = /Slice_42_output_0_DequantizeLinear_Output
builder.bind("/Slice_42_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_21);
auto input_22 = builder.wildcard(); //  id = 88  node_arg_name = /Slice_38_output_0_QuantizeLinear_Output
builder.bind("/Slice_38_output_0_QuantizeLinear_Output",input_22);
auto constant_44 = builder.constant(); //  id = 89  node_arg_name = /linear_k_9/Add_output_0_scale
builder.bind("/linear_k_9/Add_output_0_scale",constant_44);
auto constant_45 = builder.constant(); //  id = 90  node_arg_name = /Concat_174_output_0_zero_point
builder.bind("/Concat_174_output_0_zero_point",constant_45);
auto com_microsoft_DequantizeLinear_22 = builder.node2("com.microsoft:DequantizeLinear",{input_22,constant_44,constant_45}); //  id = 91  node_arg_name = /Slice_38_output_0_DequantizeLinear_Output
builder.bind("/Slice_38_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_22);
auto input_23 = builder.wildcard(); //  id = 92  node_arg_name = /Slice_34_output_0_QuantizeLinear_Output
builder.bind("/Slice_34_output_0_QuantizeLinear_Output",input_23);
auto constant_46 = builder.constant(); //  id = 93  node_arg_name = /linear_k_8/Add_output_0_scale
builder.bind("/linear_k_8/Add_output_0_scale",constant_46);
auto constant_47 = builder.constant(); //  id = 94  node_arg_name = /Concat_166_output_0_zero_point
builder.bind("/Concat_166_output_0_zero_point",constant_47);
auto com_microsoft_DequantizeLinear_23 = builder.node2("com.microsoft:DequantizeLinear",{input_23,constant_46,constant_47}); //  id = 95  node_arg_name = /Slice_34_output_0_DequantizeLinear_Output
builder.bind("/Slice_34_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_23);
auto input_24 = builder.wildcard(); //  id = 96  node_arg_name = /Slice_30_output_0_QuantizeLinear_Output
builder.bind("/Slice_30_output_0_QuantizeLinear_Output",input_24);
auto constant_48 = builder.constant(); //  id = 97  node_arg_name = /linear_k_7/Add_output_0_scale
builder.bind("/linear_k_7/Add_output_0_scale",constant_48);
auto constant_49 = builder.constant(); //  id = 98  node_arg_name = /Concat_158_output_0_zero_point
builder.bind("/Concat_158_output_0_zero_point",constant_49);
auto com_microsoft_DequantizeLinear_24 = builder.node2("com.microsoft:DequantizeLinear",{input_24,constant_48,constant_49}); //  id = 99  node_arg_name = /Slice_30_output_0_DequantizeLinear_Output
builder.bind("/Slice_30_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_24);
auto input_25 = builder.wildcard(); //  id = 100  node_arg_name = /Slice_26_output_0_QuantizeLinear_Output
builder.bind("/Slice_26_output_0_QuantizeLinear_Output",input_25);
auto constant_50 = builder.constant(); //  id = 101  node_arg_name = /Concat_150_output_0_scale
builder.bind("/Concat_150_output_0_scale",constant_50);
auto constant_51 = builder.constant(); //  id = 102  node_arg_name = /Concat_150_output_0_zero_point
builder.bind("/Concat_150_output_0_zero_point",constant_51);
auto com_microsoft_DequantizeLinear_25 = builder.node2("com.microsoft:DequantizeLinear",{input_25,constant_50,constant_51}); //  id = 103  node_arg_name = /Slice_26_output_0_DequantizeLinear_Output
builder.bind("/Slice_26_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_25);
auto input_26 = builder.wildcard(); //  id = 104  node_arg_name = /Slice_22_output_0_QuantizeLinear_Output
builder.bind("/Slice_22_output_0_QuantizeLinear_Output",input_26);
auto constant_52 = builder.constant(); //  id = 105  node_arg_name = /Concat_142_output_0_scale
builder.bind("/Concat_142_output_0_scale",constant_52);
auto constant_53 = builder.constant(); //  id = 106  node_arg_name = /Concat_142_output_0_zero_point
builder.bind("/Concat_142_output_0_zero_point",constant_53);
auto com_microsoft_DequantizeLinear_26 = builder.node2("com.microsoft:DequantizeLinear",{input_26,constant_52,constant_53}); //  id = 107  node_arg_name = /Slice_22_output_0_DequantizeLinear_Output
builder.bind("/Slice_22_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_26);
auto input_27 = builder.wildcard(); //  id = 108  node_arg_name = /Slice_18_output_0_QuantizeLinear_Output
builder.bind("/Slice_18_output_0_QuantizeLinear_Output",input_27);
auto constant_54 = builder.constant(); //  id = 109  node_arg_name = /linear_k_4/Add_output_0_scale
builder.bind("/linear_k_4/Add_output_0_scale",constant_54);
auto constant_55 = builder.constant(); //  id = 110  node_arg_name = /Concat_134_output_0_zero_point
builder.bind("/Concat_134_output_0_zero_point",constant_55);
auto com_microsoft_DequantizeLinear_27 = builder.node2("com.microsoft:DequantizeLinear",{input_27,constant_54,constant_55}); //  id = 111  node_arg_name = /Slice_18_output_0_DequantizeLinear_Output
builder.bind("/Slice_18_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_27);
auto input_28 = builder.wildcard(); //  id = 112  node_arg_name = /Slice_14_output_0_QuantizeLinear_Output
builder.bind("/Slice_14_output_0_QuantizeLinear_Output",input_28);
auto constant_56 = builder.constant(); //  id = 113  node_arg_name = /Concat_126_output_0_scale
builder.bind("/Concat_126_output_0_scale",constant_56);
auto constant_57 = builder.constant(); //  id = 114  node_arg_name = /Concat_126_output_0_zero_point
builder.bind("/Concat_126_output_0_zero_point",constant_57);
auto com_microsoft_DequantizeLinear_28 = builder.node2("com.microsoft:DequantizeLinear",{input_28,constant_56,constant_57}); //  id = 115  node_arg_name = /Slice_14_output_0_DequantizeLinear_Output
builder.bind("/Slice_14_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_28);
auto input_29 = builder.wildcard(); //  id = 116  node_arg_name = /Slice_10_output_0_QuantizeLinear_Output
builder.bind("/Slice_10_output_0_QuantizeLinear_Output",input_29);
auto constant_58 = builder.constant(); //  id = 117  node_arg_name = /Concat_118_output_0_scale
builder.bind("/Concat_118_output_0_scale",constant_58);
auto constant_59 = builder.constant(); //  id = 118  node_arg_name = /Concat_118_output_0_zero_point
builder.bind("/Concat_118_output_0_zero_point",constant_59);
auto com_microsoft_DequantizeLinear_29 = builder.node2("com.microsoft:DequantizeLinear",{input_29,constant_58,constant_59}); //  id = 119  node_arg_name = /Slice_10_output_0_DequantizeLinear_Output
builder.bind("/Slice_10_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_29);
auto input_30 = builder.wildcard(); //  id = 120  node_arg_name = /Slice_6_output_0_QuantizeLinear_Output
builder.bind("/Slice_6_output_0_QuantizeLinear_Output",input_30);
auto constant_60 = builder.constant(); //  id = 121  node_arg_name = /Concat_110_output_0_scale
builder.bind("/Concat_110_output_0_scale",constant_60);
auto constant_61 = builder.constant(); //  id = 122  node_arg_name = /Concat_110_output_0_zero_point
builder.bind("/Concat_110_output_0_zero_point",constant_61);
auto com_microsoft_DequantizeLinear_30 = builder.node2("com.microsoft:DequantizeLinear",{input_30,constant_60,constant_61}); //  id = 123  node_arg_name = /Slice_6_output_0_DequantizeLinear_Output
builder.bind("/Slice_6_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_30);
auto input_31 = builder.wildcard(); //  id = 124  node_arg_name = /Slice_2_output_0_QuantizeLinear_Output
builder.bind("/Slice_2_output_0_QuantizeLinear_Output",input_31);
auto constant_62 = builder.constant(); //  id = 125  node_arg_name = /Unsqueeze_26_output_0_scale
builder.bind("/Unsqueeze_26_output_0_scale",constant_62);
auto constant_63 = builder.constant(); //  id = 126  node_arg_name = /Concat_6_output_0_zero_point
builder.bind("/Concat_6_output_0_zero_point",constant_63);
auto com_microsoft_DequantizeLinear_31 = builder.node2("com.microsoft:DequantizeLinear",{input_31,constant_62,constant_63}); //  id = 127  node_arg_name = /Slice_2_output_0_DequantizeLinear_Output
builder.bind("/Slice_2_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_31);
auto Concat_0 = builder.node2("Concat",{com_microsoft_DequantizeLinear_31,com_microsoft_DequantizeLinear_30,com_microsoft_DequantizeLinear_29,com_microsoft_DequantizeLinear_28,com_microsoft_DequantizeLinear_27,com_microsoft_DequantizeLinear_26,com_microsoft_DequantizeLinear_25,com_microsoft_DequantizeLinear_24,com_microsoft_DequantizeLinear_23,com_microsoft_DequantizeLinear_22,com_microsoft_DequantizeLinear_21,com_microsoft_DequantizeLinear_20,com_microsoft_DequantizeLinear_19,com_microsoft_DequantizeLinear_18,com_microsoft_DequantizeLinear_17,com_microsoft_DequantizeLinear_16,com_microsoft_DequantizeLinear_15,com_microsoft_DequantizeLinear_14,com_microsoft_DequantizeLinear_13,com_microsoft_DequantizeLinear_12,com_microsoft_DequantizeLinear_11,com_microsoft_DequantizeLinear_10,com_microsoft_DequantizeLinear_9,com_microsoft_DequantizeLinear_8,com_microsoft_DequantizeLinear_7,com_microsoft_DequantizeLinear_6,com_microsoft_DequantizeLinear_5,com_microsoft_DequantizeLinear_4,com_microsoft_DequantizeLinear_3,com_microsoft_DequantizeLinear_2,com_microsoft_DequantizeLinear_1,com_microsoft_DequantizeLinear_0}); //  id = 128  node_arg_name = /Concat_355_output_0
builder.bind("/Concat_355_output_0",Concat_0);
auto constant_64 = builder.constant(); //  id = 129  node_arg_name = /Concat_355_output_0_scale
builder.bind("/Concat_355_output_0_scale",constant_64);
auto constant_65 = builder.constant(); //  id = 130  node_arg_name = /Concat_355_output_0_zero_point
builder.bind("/Concat_355_output_0_zero_point",constant_65);
auto com_microsoft_QuantizeLinear_0 = builder.node2("com.microsoft:QuantizeLinear",{Concat_0,constant_64,constant_65}); //  id = 131  node_arg_name = /Concat_355_output_0_QuantizeLinear_Output
builder.bind("/Concat_355_output_0_QuantizeLinear_Output",com_microsoft_QuantizeLinear_0);
auto constant_66 = builder.constant(); //  id = 132  node_arg_name = /Concat_355_output_0_scale
builder.bind("/Concat_355_output_0_scale",constant_66);
auto constant_67 = builder.constant(); //  id = 133  node_arg_name = /Concat_355_output_0_zero_point
builder.bind("/Concat_355_output_0_zero_point",constant_67);
auto com_microsoft_DequantizeLinear_32 = builder.node2("com.microsoft:DequantizeLinear",{com_microsoft_QuantizeLinear_0,constant_66,constant_67}); //  id = 134  node_arg_name = /Concat_355_output_0_DequantizeLinear_Output
builder.bind("/Concat_355_output_0_DequantizeLinear_Output",com_microsoft_DequantizeLinear_32);
auto constant_68 = builder.constant(); //  id = 135  node_arg_name = /Concat_357_output_0
builder.bind("/Concat_357_output_0",constant_68);
auto Reshape_0 = builder.node2("Reshape",{com_microsoft_DequantizeLinear_32,constant_68}); //  id = 136  node_arg_name = oup_cache_k_QuantizeLinear_Input
builder.bind("oup_cache_k_QuantizeLinear_Input",Reshape_0);
auto constant_69 = builder.constant(); //  id = 137  node_arg_name = /Concat_355_output_0_scale
builder.bind("/Concat_355_output_0_scale",constant_69);
auto constant_70 = builder.constant(); //  id = 138  node_arg_name = /Concat_355_output_0_zero_point
builder.bind("/Concat_355_output_0_zero_point",constant_70);
auto com_microsoft_QuantizeLinear_1 = builder.node2("com.microsoft:QuantizeLinear",{Reshape_0,constant_69,constant_70}); //  id = 139  node_arg_name = oup_cache_k_QuantizeLinear_Output
builder.bind("oup_cache_k_QuantizeLinear_Output",com_microsoft_QuantizeLinear_1);
auto constant_71 = builder.constant(); //  id = 140  node_arg_name = /Concat_355_output_0_scale
builder.bind("/Concat_355_output_0_scale",constant_71);
auto constant_72 = builder.constant(); //  id = 141  node_arg_name = /Concat_355_output_0_zero_point
builder.bind("/Concat_355_output_0_zero_point",constant_72);
auto com_microsoft_DequantizeLinear_33 = builder.node2("com.microsoft:DequantizeLinear",{com_microsoft_QuantizeLinear_1,constant_71,constant_72}); //  id = 142  node_arg_name = oup_cache_k
builder.bind("oup_cache_k",com_microsoft_DequantizeLinear_33);
ret = com_microsoft_DequantizeLinear_33;
