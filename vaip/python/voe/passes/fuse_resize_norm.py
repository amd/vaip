##
##  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
##  Licensed under the MIT License.
##
import sys

import glog as log
import numpy as np
from voe.anchor_point import CONST
from voe.pattern import node, wildcard, xir_const_op
from voe.rule_ext import Rule, same_as


class fuse_resize_normalize(Rule):
    def pattern(self):
        img = wildcard()
        fdpre = node("vitis.customop:ResizeNormalize", img)
        return fdpre.build(locals())

    # argument is passed by key, value pair
    # so the argument name should be identical to the pattern's local variable's name
    def action(self, img, fdpre, **kwargs):
        inputs = [img]
        outputs = [fdpre]
        # pattern
        meta_def = self.try_fuse("ResizeNormalize", inputs, outputs, [], "ResizeNorm")
        meta_def.set_generic_param("input_shape", str(img.shape()))
        meta_def.set_generic_param("output_shape", str(fdpre.shape()))
        meta_def.set_generic_param("transpose", str(fdpre.attr("transpose")))
        meta_def.set_generic_param("alpha_fbits", str(fdpre.attr("alpha_fl")))
        meta_def.set_generic_param("beta_fbits", str(fdpre.attr("beta_fl")))
        meta_def.set_generic_param("output_fbits", str(fdpre.attr("out_fl")))
        meta_def.set_generic_param("Mean", str(fdpre.attr("mean")))
        meta_def.set_generic_param("StdDev", str(fdpre.attr("stddev")))
        meta_def.set_generic_param("size", str(fdpre.attr("out_shape")))
        meta_def.fuse()


def rules():
    return [fuse_resize_normalize()]
