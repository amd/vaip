##
##  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc.
##
##  Licensed under the Apache License, Version 2.0 (the "License");
##  you may not use this file except in compliance with the License.
##  You may obtain a copy of the License at
##
##  http://www.apache.org/licenses/LICENSE-2.0
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.
##
def quantize_static(input_model_path, output_model_path, input_shape, quantize_mode):
    if input_shape[0] == -1:
        input_shape[0] = 1

    if quantize_mode == "vaiq":
        from vai_q_onnx.quantize import quantize_static

        quantize_static(input_model_path, output_model_path, None, input_shape)
    else:
        from olive.model import ONNXModel
        from olive.passes.onnx import OnnxStaticQuantization
        from vai_q_onnx.calibrate import RandomDataReader

        dataloader = lambda a, b: RandomDataReader(input_model_path, input_shape)
        quantization = OnnxStaticQuantization(
            {
                "dataloader_func": dataloader,
                "ActivationSymmetric": True,
            },
            True,
        )
        model = ONNXModel(input_model_path, "resnet18")
        quantization.run(model, output_model_path)
