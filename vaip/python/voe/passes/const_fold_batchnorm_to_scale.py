##
##  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
##  Licensed under the MIT License.
##

import sys

import glog as log
import numpy as np
from voe.pattern import node, wildcard, xir_const_op
from voe.rule_ext import Rule, same_as

"""
  test case model 5

  Replace pattern pass
  X : wildcard()
  From : batchnorm(input=*, gamma=fix(const()), beta=fix(const()),
                   moving_mean=const().where(all_zeros()), moving_var=const().where(all_ones()))

      output = (input - moving_mean) /
              sqrt(moving_var + epsilon) * gamma + beta

  To  : scale(input, weights, bias)

  where weights and bias are `fix(const())`
"""


class ConstantFoldBatchnormToScale(Rule):
    def pattern(self):
        input = wildcard()
        gamma = node("com.xilinx:fix", xir_const_op())
        beta = node("com.xilinx:fix", xir_const_op())
        moving_mean = xir_const_op()
        moving_var = xir_const_op()
        batchnorm = node(
            "com.xilinx:batchnorm", input, gamma, beta, moving_mean, moving_var
        )
        return batchnorm.build(locals())

    def where(self, batchnorm, moving_mean, moving_var, **_rest):
        print(f"moving_var = {str(moving_var)}")
        moving_var_data = moving_var.const_data()
        if not np.all(np.asarray(moving_var_data) == 1.0):
            log.info(
                "moving_var are not all ones. it is not supported yet."
                " cancel constant folding batchnorm"
            )
            return False

        moving_mean_data = moving_mean.const_data()
        if not np.all(np.asarray(moving_mean_data) == 0.0):
            log.info(
                "moving_mean are not all zeros. it is not supported yet."
                " cancel constant folding batchnorm"
            )
            return False

        epsilon = batchnorm.attr("epsilon")

        if epsilon != 0.0:
            log.info(
                "epision is not zero, it is not supported yet. "
                "cancel constant folding batchnorm"
            )
            return False
        return True

    def action(self, input, gamma, beta, batchnorm, **_others):
        return self.create_node(
            op_type="com.xilinx:scale",
            inputs=[input, gamma, beta],
            data_type=same_as(batchnorm),
            shape=same_as(batchnorm),
            attrs={"axis": batchnorm.attr("axis")},
            anchor_point=batchnorm,
        )


def rules():
    return [ConstantFoldBatchnormToScale()]
