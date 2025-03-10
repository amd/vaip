##
##  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
##  Licensed under the MIT License.
##

from voe.pattern import node, wildcard


def pattern():
    dequantize_x = wildcard()
    dequantize_x_scale = wildcard()
    dequantize_x_zero_point = wildcard()
    X = node(
        "DequantizeLinear",
        dequantize_x,
        dequantize_x_scale,
        [dequantize_x_zero_point],
    )

    dequantize_x2 = wildcard()
    dequantize_x_scale2 = wildcard()
    dequantize_x_zero_point2 = wildcard()
    W = node(
        "DequantizeLinear",
        dequantize_x2,
        dequantize_x_scale2,
        [dequantize_x_zero_point2],
    )

    dequantize_x3 = wildcard()
    dequantize_x_scale3 = wildcard()
    dequantize_x_zero_point3 = wildcard()
    B = node(
        "DequantizeLinear",
        dequantize_x3,
        dequantize_x_scale3,
        [dequantize_x_zero_point3],
    )

    conv = node("Conv", X, W, [B])

    quantize_y_scale4 = wildcard()
    quantize_y_zp4 = wildcard()
    y4 = node("QuantizeLinear", conv, quantize_y_scale4, [quantize_y_zp4])
    return y4.build(locals())
