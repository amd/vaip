##
##  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
##  Licensed under the MIT License.
##

from voe.pattern import node, wildcard, xir_const_op
from voe.rule_ext import Node, Rule, same_as

"""
  test case model 112

  remove reshape fix
  From : fix(reshape(fix(input),[shape]))
  To  : reshape(input,[shape])
"""


class RemoveReshapeFix(Rule):
    def pattern(self):
        input = wildcard()
        before_fix = node("com.xilinx:fix", input)
        shape = xir_const_op()
        reshape = node("com.xilinx:reshape", before_fix, [shape])
        after_fix = node("com.xilinx:fix", reshape)
        return after_fix.build(locals())

    def where(self, before_fix: Node, after_fix: Node, **kwargs):
        if (
            len(after_fix.get_consumers()) == 1
            and "concat" == after_fix.get_consumers()[0].op_type()
        ):
            return before_fix.attr("fix_point") == after_fix.attr("fix_point")
        return False

    def action(self, reshape: Node, input: Node, after_fix: Node, **kwargs):
        if not "shape" in kwargs:
            inputs = [input]
        else:
            inputs = [input, kwargs["shape"]]
        return self.create_node(
            op_type=same_as(reshape),
            inputs=inputs,
            attrs=same_as(reshape),
            anchor_point=after_fix,
        )


def rules():
    return [RemoveReshapeFix()]
