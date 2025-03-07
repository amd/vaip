##
##  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
##  Licensed under the MIT License.
##

import argparse

import numpy as np
import onnx


def onnx_type_to_numpy_type(onnx_type):
    mapping = {
        onnx.TensorProto.FLOAT: np.float32,
        onnx.TensorProto.UINT8: np.uint8,
        onnx.TensorProto.INT8: np.int8,
        onnx.TensorProto.UINT16: np.uint16,
        onnx.TensorProto.INT16: np.int16,
        onnx.TensorProto.INT32: np.int32,
        onnx.TensorProto.INT64: np.int64,
        onnx.TensorProto.FLOAT16: np.float16,
        onnx.TensorProto.DOUBLE: np.float64,
        onnx.TensorProto.COMPLEX64: np.complex64,
        onnx.TensorProto.COMPLEX128: np.complex128,
        onnx.TensorProto.UINT32: np.uint32,
        onnx.TensorProto.UINT64: np.uint64,
    }
    return mapping.get(onnx_type, None)


parser = argparse.ArgumentParser(description="Extract raw_data from TensorProto")
parser.add_argument(
    "filename", type=str, help="Path to the file containing tensor data"
)
parser.add_argument("output", type=str, help="Path to save the raw_data as binary file")
parser.add_argument("--nchw2nhwc", action="store_true", help="Enable nchw2nhwc mode")
args = parser.parse_args()

with open(args.filename, "rb") as f:
    tensor_data = f.read()

tensor_proto = onnx.TensorProto()
with open(args.filename, "rb") as f:
    tensor_proto.ParseFromString(f.read())

data_type_str = onnx.TensorProto.DataType.Name(tensor_proto.data_type)

if len(tensor_proto.dims) != 4:
    with open(args.output, "wb") as f:
        f.write(tensor_proto.raw_data)
    print(f"{args.filename} data_type is {data_type_str}, dims is{tensor_proto.dims}")
else:
    if args.nchw2nhwc:
        data_type = tensor_proto.data_type
        raw_data = tensor_proto.raw_data
        shape = tensor_proto.dims
        np_type = onnx_type_to_numpy_type(tensor_proto.data_type)
        # Convert raw_data to a NumPy array based on the data_type
        if np_type:
            numpy_array = np.frombuffer(raw_data, dtype=np_type).reshape(shape)
            numpy_array = np.transpose(numpy_array, axes=(0, 2, 3, 1))
            numpy_array.tofile(args.output)
            print(
                f"{args.filename} data_type is {data_type_str}, new dims is{numpy_array.shape}"
            )
        else:
            print(f"not support data_type : {data_type_str}")
            exit(-1)
    else:
        with open(args.output, "wb") as f:
            f.write(tensor_proto.raw_data)
        print(
            f"{args.filename} data_type is {data_type_str}, dims is{tensor_proto.dims}"
        )
