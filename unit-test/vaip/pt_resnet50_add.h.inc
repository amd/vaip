// Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
// Licensed under the MIT License.
/** generated by the following command:
env \
 IGNORE_CONSTANT=1 \
 ENABLE_CONSTNAT_SHARING=1 \
 $BUILD/vaip/onnxruntime_vitisai_ep/onnx_pattern_gen \
 -i 220 \
 -i 267 \
 -o 287 \
 -f /workspace/test_onnx_runner/data/pt_resnet50.onnx\
 -c /workspace/vaip/unit-test/vaip/src/pt_resnet50_add.h.inc
*/
auto constant_0 = builder.constant(); //  id = 0  node_arg_name = module_17.bias
builder.bind("module_17.bias",constant_0);
auto constant_1 = builder.constant(); //  id = 1  node_arg_name = 274
builder.bind("274",constant_1);
auto constant_2 = builder.constant(); //  id = 2  node_arg_name = 275
builder.bind("275",constant_2);
auto QuantizeLinear_0 = builder.node2("QuantizeLinear",{constant_0,constant_1,constant_2}); //  id = 3  node_arg_name = 276
builder.bind("276",QuantizeLinear_0);
auto constant_3 = builder.constant(); //  id = 4  node_arg_name = 277
builder.bind("277",constant_3);
auto constant_4 = builder.constant(); //  id = 5  node_arg_name = 278
builder.bind("278",constant_4);
auto DequantizeLinear_0 = builder.node2("DequantizeLinear",{QuantizeLinear_0,constant_3,constant_4}); //  id = 6  node_arg_name = 279
builder.bind("279",DequantizeLinear_0);
auto constant_5 = builder.constant(); //  id = 7  node_arg_name = module_17.weight
builder.bind("module_17.weight",constant_5);
auto constant_6 = builder.constant(); //  id = 8  node_arg_name = 268
builder.bind("268",constant_6);
auto constant_7 = builder.constant(); //  id = 9  node_arg_name = 269
builder.bind("269",constant_7);
auto QuantizeLinear_1 = builder.node2("QuantizeLinear",{constant_5,constant_6,constant_7}); //  id = 10  node_arg_name = 270
builder.bind("270",QuantizeLinear_1);
auto constant_8 = builder.constant(); //  id = 11  node_arg_name = 271
builder.bind("271",constant_8);
auto constant_9 = builder.constant(); //  id = 12  node_arg_name = 272
builder.bind("272",constant_9);
auto DequantizeLinear_1 = builder.node2("DequantizeLinear",{QuantizeLinear_1,constant_8,constant_9}); //  id = 13  node_arg_name = 273
builder.bind("273",DequantizeLinear_1);
auto input_0 = builder.wildcard(); //  id = 14  node_arg_name = 267
builder.bind("267",input_0);
auto Conv_0 = builder.node2("Conv",{input_0,DequantizeLinear_1,DequantizeLinear_0}); //  id = 15  node_arg_name = 280
builder.bind("280",Conv_0);
auto constant_10 = builder.constant(); //  id = 16  node_arg_name = 281
builder.bind("281",constant_10);
auto constant_11 = builder.constant(); //  id = 17  node_arg_name = 282
builder.bind("282",constant_11);
auto QuantizeLinear_2 = builder.node2("QuantizeLinear",{Conv_0,constant_10,constant_11}); //  id = 18  node_arg_name = 283
builder.bind("283",QuantizeLinear_2);
auto constant_12 = builder.constant(); //  id = 19  node_arg_name = 284
builder.bind("284",constant_12);
auto constant_13 = builder.constant(); //  id = 20  node_arg_name = 285
builder.bind("285",constant_13);
auto DequantizeLinear_2 = builder.node2("DequantizeLinear",{QuantizeLinear_2,constant_12,constant_13}); //  id = 21  node_arg_name = 286
builder.bind("286",DequantizeLinear_2);
auto input_1 = builder.wildcard(); //  id = 22  node_arg_name = 220
builder.bind("220",input_1);
auto Relu_0 = builder.node2("Relu",{input_1}); //  id = 23  node_arg_name = 221
builder.bind("221",Relu_0);
auto constant_14 = builder.constant(); //  id = 24  node_arg_name = 222
builder.bind("222",constant_14);
auto constant_15 = builder.constant(); //  id = 25  node_arg_name = 223
builder.bind("223",constant_15);
auto QuantizeLinear_3 = builder.node2("QuantizeLinear",{Relu_0,constant_14,constant_15}); //  id = 26  node_arg_name = 224
builder.bind("224",QuantizeLinear_3);
auto constant_16 = builder.constant(); //  id = 27  node_arg_name = 225
builder.bind("225",constant_16);
auto constant_17 = builder.constant(); //  id = 28  node_arg_name = 226
builder.bind("226",constant_17);
auto DequantizeLinear_3 = builder.node2("DequantizeLinear",{QuantizeLinear_3,constant_16,constant_17}); //  id = 29  node_arg_name = 227
builder.bind("227",DequantizeLinear_3);
auto Add_0 = builder.node2("Add",{DequantizeLinear_2,DequantizeLinear_3}); //  id = 30  node_arg_name = 287
builder.bind("287",Add_0);
ret = Add_0;
