/*
 *     The Xilinx Vitis AI Vaip in this distribution are provided under the
 * following free and permissive binary-only license, but are not provided in
 * source code form.  While the following free and permissive license is similar
 * to the BSD open source license, it is NOT the BSD open source license nor
 * other OSI-approved open source license.
 *
 *      Copyright (C) 2022 Xilinx, Inc. All rights reserved.
 *      Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights
 * reserved.
 *
 *      Redistribution and use in binary form only, without modification, is
 * permitted provided that the following conditions are met:
 *
 *      1. Redistributions must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 *      2. The name of Xilinx, Inc. may not be used to endorse or promote
 * products redistributed with this software without specific prior written
 * permission.
 *
 *      THIS SOFTWARE IS PROVIDED BY XILINX, INC. "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL XILINX, INC. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *      PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 */
DEF_ENV_PARAM(XLNX_ENABLE_OLD_QDQ, "1")
static std::unique_ptr<BaseRule> DequantizeLinear(IPass& pass) {
  return std::make_unique<ConstantFoldRule>(
      pass, "DequantizeLinear",
      [](IPass& self, const Node& node, GTensorView<float> output,
         GTensorView<int8_t> input, float scale,
         std::optional<int8_t> zero_point) -> bool {
        if (ENV_PARAM(XLNX_ENABLE_OLD_QDQ) == 0) {
          return false;
        }
        auto fix_point_p = scale_to_fix_point(scale);
        if (fix_point_p.get() == nullptr) {
          return false;
        }
        auto fix_point = *fix_point_p;
        auto node_arg_name = node_get_output_name(node);
        self.set_fix_info(node_arg_name.c_str(), fix_point);
        MY_LOG(2) << "scale " << scale << " "                    //
                  << "input.size " << input.data.size() << " "   //
                  << "output.size " << output.data.size() << " " //
                  << "fix_point " << fix_point << " "            //
            ;
        CHECK_EQ(output.data.size(), input.data.size());
        auto zero = zero_point.value_or(0);
        auto size = output.data.size();
        for (auto i = 0u; i < size; ++i) {
          output.data[i] = ((float)(input.data[i] + zero)) * scale;
        }
        return true;
      });
}
static std::unique_ptr<BaseRule> DequantizeLinear_int32_t(IPass& pass) {
  return std::make_unique<ConstantFoldRule>(
      pass, "DequantizeLinear",
      [](IPass& self, const Node& node, GTensorView<float> output,
         GTensorView<int32_t> input, float scale,
         std::optional<int32_t> zero_point) -> bool {
        if (ENV_PARAM(XLNX_ENABLE_OLD_QDQ) == 0) {
          return false;
        }
        auto fix_point_p = scale_to_fix_point(scale);
        if (fix_point_p.get() == nullptr) {
          return false;
        }
        auto fix_point = *fix_point_p;
        auto node_arg_name = node_get_output_name(node);
        self.set_fix_info(node_arg_name.c_str(), fix_point);
        MY_LOG(2) << "scale " << scale << " "                    //
                  << "input.size " << input.data.size() << " "   //
                  << "output.size " << output.data.size() << " " //
                  << "fix_point " << fix_point << " "            //
            ;
        CHECK_EQ(output.data.size(), input.data.size());
        auto zero = zero_point.value_or(0);
        auto size = output.data.size();
        for (auto i = 0u; i < size; ++i) {
          output.data[i] = ((float)(input.data[i] + zero)) * scale;
        }
        return true;
      });
}