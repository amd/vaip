##
##  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
##  Licensed under the MIT License.
##
from ..node import Node
from .util import isa_SameAs, same_as


class NodeBuilderProxy:
    def __init__(
        self,
        rule,
        builder,
        op_type=None,
        inputs=[],
        attrs={},
        data_type=None,
        shape=None,
        anchor_point=None,
        data=None,
    ):
        self._rule = rule
        self._builder = builder
        self._op_type = op_type
        self._inputs = inputs
        self._attrs = attrs
        self._data_type = data_type
        self._shape = shape
        self._anchor_point = anchor_point
        self._data = data

    def _compile_op_type(self):
        if self._op_type is not None:
            op_type = self._op_type
            if isa_SameAs(op_type):
                node = op_type.value()
                self._builder.clone_op_type(node)
            elif isinstance(op_type, str):
                op_types = op_type.split(":")
                domain = "ai.onnx"
                if op_types.__len__() == 1:
                    op_type = op_types[0]
                elif op_types.__len__() == 2:
                    domain = op_types[0]
                    op_type = op_types[1]
                else:
                    raise TypeError(f"not supported op_type : {op_type}")
                return self._builder.set_op_type(op_type, domain)
            else:
                raise TypeError(f"not supported op_type : {op_type}")
        else:
            raise TypeError(f"cannot find `op_type`")

    def _compile_inputs(self):
        if self._inputs is not None:
            inputs = self._inputs
            if isa_SameAs(inputs):
                node = inputs.value()
                return self._builder.clone_inputs(node)
            else:
                input_ops = []
                for input in inputs:
                    if isinstance(input, Node):
                        input_ops.append(input)
                    else:
                        raise TypeError(f"not supported input : {input}")
                return self._builder.set_input_args(input_ops)
        else:
            return []

    def _compile_attr_kv(self, attr_name, attr_value):
        self._builder.set_attr(attr_name, attr_value)

    def _compile_attrs(self):
        """attrs is optional"""
        if self._attrs is not None:
            attrs = self._attrs
            if isa_SameAs(attrs):
                node = attrs.value()
                return self._builder.clone_attrs(node)
            elif isinstance(attrs, dict):
                return [self._compile_attr_kv(k, v) for (k, v) in attrs.items()]
            else:
                raise TypeError(f"not supported attrs: {attrs}")
        else:
            return []

    def _compile_data_type(self):
        """data_type is optional"""
        if self._data_type is not None:
            data_type = self._data_type
            if isa_SameAs(data_type):
                node = data_type.value()
                return self._builder.clone_data_type(node)
            elif isinstance(data_type, str):
                return self._builder.set_data_type(data_type)
            else:
                raise TypeError("not supported set_data_type")
        else:
            return []

    def _compile_shape(self):
        """shape is optional"""
        if self._shape is not None:
            shape = self._shape
            if isa_SameAs(shape):
                node = shape.value()
                return self._builder.clone_shape(node)
            elif isinstance(shape, list):
                return self._builder.set_shape(shape)
            else:
                raise TypeError("not supported set_shape")
        else:
            return []

    def _compile_anchor_point(self):
        if self._anchor_point is not None:
            anchor_point = self._anchor_point
            if isinstance(anchor_point, Node):
                node = anchor_point
                return self._builder.set_anchor_point1(node)
            elif isinstance(anchor_point, tuple):
                if len(anchor_point) == 2:
                    return self._builder.set_anchor_point2(
                        anchor_point[0], anchor_point[1]
                    )
                elif len(anchor_point) == 3:
                    return self._builder.set_anchor_point3(
                        anchor_point[0], anchor_point[1], anchor_point[2]
                    )
                else:
                    raise TypeError(f"not support anchor_point : {anchor_point}")
            else:
                raise TypeError(f"not support anchor_point : {anchor_point}")
        else:
            raise TypeError(f"cannot find `anchor_point`")

    def build(self):
        """return Node"""
        self._compile_op_type()
        self._compile_inputs()
        self._compile_attrs()
        self._compile_data_type()
        self._compile_shape()
        self._compile_anchor_point()
