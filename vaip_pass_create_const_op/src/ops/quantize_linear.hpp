/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

static std::unique_ptr<BaseRule> QuantizeLinear(IPass& pass) {
  return std::make_unique<ConstantFoldRule>(
      pass, "QuantizeLinear",
      [](IPass& self, const Node& node, GTensorView<int8_t> output,
         GTensorView<float> input, float scale,
         std::optional<int8_t> zero_point) -> bool {
        auto fix_point_p = scale_to_fix_point(scale);
        if (fix_point_p.get() == nullptr) {
          return false;
        }
        auto fix_point = *fix_point_p;
        auto node_arg_name = node_get_output_name(node);
        self.set_fix_info(node_arg_name.c_str(), fix_point);
        CHECK_EQ(output.data.size(), input.data.size());
        auto zero = zero_point.value_or(0.0f);
        auto size = output.data.size();
        for (auto i = 0u; i < size; ++i) {
          auto y = input.data[i] / scale + zero;
          if (y >= 127.0f) {
            output.data[i] = 127;
          } else if (y <= -128.0f) {
            output.data[i] = -128;
          } else {
            output.data[i] = (int8_t)(y);
          }
        }
        return true;
      });
}
