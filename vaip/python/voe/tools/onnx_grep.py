##
##  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
##  Licensed under the MIT License.
##
from voe.model import *
from voe.pattern import *
import onnxruntime


def get_pattern(filename):
    file_str = open(filename, "r").read()
    pattern_builder = PatternBuilder()
    if filename.endswith(".py"):
        return pattern_builder.create_pattern_by_py(file_str)
    elif filename.endswith(".json"):
        return pattern_builder.create_by_json(file_str)
    else:
        raise RuntimeError("Unsupported pattern data type")


def onnx_grep(args):
    onnxruntime.initialize_session(
        providers=["VitisAIExecutionProvider"],
        provider_options=[{"config_file": args.config_file}],
    )
    p = get_pattern(args.pattern)
    print(p.__str__())
    model = Model(args.model)
    graph = model.get_main_graph()
    graph.resolve(True)
    index = graph.get_node_in_topoligical_order()
    for i in index:
        node = graph.get_node(i)
        binder = p.match(node)
        if binder != None:
            print(f"find node: {node.__str__()}")


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--model", type=str)
    parser.add_argument("-p", "--pattern", type=str)
    parser.add_argument("config_file", type=str)

    args = parser.parse_args()

    if not args.model:
        raise RuntimeError("no model provided")
    if not args.pattern:
        raise RuntimeError("no pattern provided")
    onnx_grep(args)
