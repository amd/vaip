##
##  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
##  Licensed under the MIT License.
##
import sys
import string
import os

# import glog as log
import numpy as np
from voe.anchor_point import CONST
from voe.pattern import node, wildcard, xir_const_op
from voe.rule_ext import Rule, same_as


class fuse_MATMUL(Rule):
    def pattern(self):
        self.num = 0
        p_input = wildcard()
        input_w = wildcard()

        matmul = node("MatMul", p_input, input_w)
        return matmul.build(locals())

    # argument is passed by key, value pair
    # so the argument name should be identical to the pattern's local variable's name
    def action(self, p_input, input_w, matmul, **kwargs):
        self.num = self.num + 1

        inputs = [p_input]
        outputs = [matmul]

        if input_w.is_constant():
            name = input_w.__str__()
            valid_chars = "-_.() %s%s" % (string.ascii_letters, string.digits)
            filename = "".join(c for c in name if c in valid_chars)
            name = filename.replace(" ", "_")  # remove spaces in filenames.
            wts_bin = self.cache_dir() + "/" + str(name) + "_weight.bin"

            weight_f = np.array(input_w.const_data(), dtype=np.single).reshape(
                input_w.shape()[0], input_w.shape()[1]
            )
            weight_scale = (np.abs(weight_f).max()) / 128
            weight_q = np.clip(np.round(weight_f / weight_scale), -128, 127).astype(
                np.int8
            )
            # print(weight_man.sum(), weight_man.shape)
            file = open(wts_bin, "wb")
            file.write(np.array(weight_q).tobytes())
            file.close()

            meta_def = self.try_fuse("MATMUL_" + str(name), inputs, outputs, [], "GEMM")
            meta_def.set_generic_param("wts_shape_dim_0", str(input_w.shape()[1]))
            meta_def.set_generic_param("wts_shape_dim_1", str(input_w.shape()[0]))

            meta_def.set_generic_param("node_name", str(name))
            meta_def.set_generic_param("wts_file", str(wts_bin))
            meta_def.set_generic_param("wts_scale", str(weight_scale))
            meta_def.set_generic_param("bias_file", str("null"))
            meta_def.set_generic_param("cache_dir", str(self.cache_dir()))

            return meta_def.fuse()


def rules():
    return [fuse_MATMUL()]
