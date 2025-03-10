##
##  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
##  Licensed under the MIT License.
##
import sys

from voe.anchor_point import NCHW2NHWC
from voe.pattern import graph_input, node, wildcard

## test case : # 100


def transpose_shape(shape, order):
    return [shape[o] for o in order]


def transpose_paddings(paddings, order):
    new_paddings = []
    for o in order:
        new_paddings.append(paddings[2 * o])
        new_paddings.append(paddings[2 * o + 1])
    return new_paddings


def action(vaip_pass, graph, transpose, fix, pad, fix1):
    fix1_shape = fix1.node().outputs()[0].shape()
    order = transpose.node().attr_ints("order")
    paddings = pad.node().attr_ints("paddings")
    new_shape = transpose_shape(fix1_shape, order)
    new_paddings = transpose_paddings(paddings, order)

    # yapf: disable
    (graph.builder(vaip_pass)
     .clone_op_type(fix.node()) # fix
     .set_input_nodes([graph.builder(vaip_pass)
                       .clone_op_type(pad.node()) # pad
                       .set_input_nodes([
                                         graph.builder(vaip_pass)
                                         .clone_op_type(fix1.node()) # fix
                                         .set_input_nodes([
                                             graph.builder(vaip_pass)
                                             .clone_op_type(transpose.node()) # transpose
                                             .clone_inputs(fix1.node())
                                             .clone_attrs(transpose.node())
                                             .clone_data_type(fix1.node())
                                             .set_anchor_point3(fix1.node(),NCHW2NHWC,new_shape)
                                             .build()])
                                         .clone_attrs(fix1.node())
                                         .clone_data_type(transpose.node())
                                         .set_anchor_point3(fix1.node(),NCHW2NHWC, new_shape)
                                         .build()])
                       .clone_attrs(pad.node())
                       .add_ints("paddings", new_paddings)
                       .clone_data_type(transpose.node())
                       .clone_shape(transpose.node())
                       .set_anchor_point2(pad.node(), NCHW2NHWC)
                       .build()])
     .clone_attrs(fix.node())
     .set_anchor_point1(transpose.node())
     .build()
     )
    return True


def pattern():
    fix1 = node("com.xilinx:fix", graph_input())
    pad = node("com.xilinx:pad", fix1)
    fix = node("com.xilinx:fix", pad)
    transpose = node("com.xilinx:transpose", fix)
    return transpose.build(locals())


def process(vaip_pass, graph):
    # yapf: disable
    return (pattern(), action)
