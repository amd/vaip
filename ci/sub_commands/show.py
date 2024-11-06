#!/usr/bin/env python
# coding=utf-8

#
#   The Xilinx Vitis AI Vaip in this distribution are provided under the following free
#   and permissive binary-only license, but are not provided in source code form.  While the following free
#   and permissive license is similar to the BSD open source license, it is NOT the BSD open source license
#   nor other OSI-approved open source license.
#
#    Copyright (C) 2022 Xilinx, Inc. All rights reserved.
#    Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
#
#    Redistribution and use in binary form only, without modification, is permitted provided that the following conditions are met:
#
#    1. Redistributions must reproduce the above copyright notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the distribution.
#
#    2. The name of Xilinx, Inc. may not be used to endorse or promote products redistributed with this software without specific
#    prior written permission.
#
#    THIS SOFTWARE IS PROVIDED BY XILINX, INC. "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL XILINX, INC.
#    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
#    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
#


import argparse
import json
import logging

import tools.vaip_model_recipe as recipe
from tools import workspace as workspace


def show(ref0):
    print(f"id:\t\t\t{ref0['id']}")
    print(f"result:\t\t\t{ref0['result']}")
    # print(f"md5sum:\t\t\t{ref0['md5sum']}")
    print(f"onnx_model:\t\t{ref0['detail']['onnx_model']}")
    print(f"result_run_onnx:\t{ref0['detail']['result_run_onnx']}")
    print(f"result_compile_onnx:\t{ref0['detail']['result_compile_onnx']}")
    print(f"result_download_onnx:\t{ref0['detail']['result_download_onnx']}")


def main(args):
    w = workspace.Workspace()
    all_recipes = recipe.create_receipes_from_args(w, args)
    if hasattr(args, "case") and args.case:
        for key in args.case:
            ref0 = w._ref_json[key]
            show(ref0)
    else:
        for k in sorted(w._ref_json.keys()):
            ref0 = w._ref_json[k]
            if ref0.get("result", "FAILED") == "OK":
                continue
            show(ref0)


def help(subparsers):
    parser = subparsers.add_parser("show", help="show the test result in reference")
    parser.add_argument("case", nargs="*", help="run case with id(could be multiple)")
    parser.add_argument(
        "-e", "--env", nargs="*", help="read enviroment variables from a file"
    )
    parser.set_defaults(func=main)
