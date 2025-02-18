##
##  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
##  Licensed under the MIT License.
##
import sys

import glog as log
import numpy as np
from voe.anchor_point import FIX, NCCHW2NHWCC, NCHW2NHWC
from voe.pattern import node, wildcard
from voe.rule_ext import Rule, same_as

"""
  test case model hp_FDM1
  From : reshape(fix2float(float2fix(transpose(fix2float(float2fix(reshape(input)))))))
  To  : transpose(reshape(fix2float(float2fix(transpose(fix2float(float2fix(reshape(transpose(input)))))))))
"""


def convert_shape(shape, order):
    return [shape[i] for i in order]


class AddTransposeAoundReshapeTransposeReshape(Rule):
    def where(
        self,
        input,
        reshape_1,
        float2fix_1,
        fix2float_1,
        transpose,
        fix2float_2,
        float2fix_2,
        reshape_2,
        **kwargs
    ):
        input_shape = input.shape()
        if not len(input_shape) == 4:
            return False

        reshape_1_shape = reshape_1.shape()
        if not len(reshape_1_shape) == 5 or not reshape_1_shape[1] == 2:
            return False

        transpose_order = transpose.attr("order")
        if not transpose_order == [0, 2, 1, 3, 4]:
            return False

        float2fix_2_shape = float2fix_2.shape()
        reshape_2_shape = reshape_2.shape()
        if not len(reshape_2_shape) == 4 or not reshape_2_shape[1] == (
            float2fix_2_shape[1] * float2fix_2_shape[2]
        ):
            return False

        return True

    def action(
        self,
        input,
        reshape_1,
        float2fix_1,
        fix2float_1,
        transpose,
        fix2float_2,
        float2fix_2,
        reshape_2,
        **kwargs
    ):
        order = [0, 2, 3, 1]

        convert_top_transpose_shape = convert_shape(input.shape(), order)
        top_transpose = self.create_node(
            op_type="com.xilinx:transpose",
            inputs=same_as(reshape_1),
            attrs={"order": order},
            data_type=same_as(input),
            anchor_point=(input, NCHW2NHWC, convert_top_transpose_shape),
        )

        convert_shape_1 = convert_shape(reshape_1.shape(), [0, 3, 4, 1, 2])
        new_reshape_1 = self.create_node(
            op_type=same_as(reshape_1),
            inputs=[top_transpose],
            data_type=same_as(reshape_1),
            anchor_point=(reshape_1, NCCHW2NHWCC, convert_shape_1),
        )

        new_float2fix_1 = self.create_node(
            op_type=same_as(float2fix_1),
            inputs=[new_reshape_1],
            attrs=same_as(float2fix_1),
            data_type=same_as(float2fix_1),
            anchor_point=(float2fix_1, NCCHW2NHWCC, convert_shape_1),
        )
        new_fix2float_1 = self.create_node(
            op_type=same_as(fix2float_1),
            inputs=[new_float2fix_1],
            attrs=same_as(fix2float_1),
            data_type=same_as(fix2float_1),
            anchor_point=(fix2float_1, NCCHW2NHWCC, convert_shape_1),
        )

        convert_transpose_shape = convert_shape(transpose.shape(), [0, 3, 4, 1, 2])
        new_transpose = self.create_node(
            op_type=same_as(transpose),
            inputs=[new_fix2float_1],
            attrs={"order": [0, 1, 2, 4, 3]},
            data_type=same_as(transpose),
            anchor_point=(transpose, NCCHW2NHWCC, convert_transpose_shape),
        )

        new_float2fix_2 = self.create_node(
            op_type=same_as(float2fix_2),
            inputs=[new_transpose],
            attrs=same_as(float2fix_2),
            data_type=same_as(float2fix_2),
            anchor_point=(float2fix_2, NCCHW2NHWCC, convert_transpose_shape),
        )

        new_fix2float_2 = self.create_node(
            op_type=same_as(fix2float_2),
            inputs=[new_float2fix_2],
            attrs=same_as(fix2float_2),
            data_type=same_as(fix2float_2),
            anchor_point=(fix2float_2, NCCHW2NHWCC, convert_transpose_shape),
        )

        convert_reshape_2_shape = convert_shape(reshape_2.shape(), order)
        new_reshape_2 = self.create_node(
            op_type=same_as(reshape_2),
            inputs=[new_fix2float_2],
            data_type=same_as(reshape_2),
            anchor_point=(reshape_2, NCHW2NHWC, convert_reshape_2_shape),
        )

        bottom_transpose = self.create_node(
            op_type="com.xilinx:transpose",
            inputs=[new_reshape_2],
            attrs={"order": [0, 3, 1, 2]},
            data_type=same_as(reshape_2),
            shape=same_as(reshape_2),
            anchor_point=reshape_2,
        )

        return bottom_transpose

    def pattern(self):
        input = wildcard()
        reshape_1 = node("com.xilinx:reshape", input)
        float2fix_1 = node("com.xilinx:float2fix", reshape_1)
        fix2float_1 = node("com.xilinx:fix2float", float2fix_1)
        transpose = node("com.xilinx:transpose", fix2float_1)
        float2fix_2 = node("com.xilinx:float2fix", transpose)
        fix2float_2 = node("com.xilinx:fix2float", float2fix_2)
        reshape_2 = node("com.xilinx:reshape", fix2float_2)
        return reshape_2.build(locals())


def rules():
    return [AddTransposeAoundReshapeTransposeReshape()]
