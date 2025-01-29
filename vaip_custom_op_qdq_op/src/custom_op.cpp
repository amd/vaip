/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#include "onnxruntime_api.hpp"

#include <glog/logging.h>
#include <sstream>
//
#include "./custom_op.hpp"

#define _QDQ_MT_ 0

namespace vaip_qdq_op_custom_op {

MyCustomOp::MyCustomOp(std::shared_ptr<const PassContext> context,
                       const std::shared_ptr<MetaDefProto>& meta_def,
                       onnxruntime::Model* model)
    : CustomOpImp(context, meta_def, model) {
  // std::cout << " Custom QDQ_OP constructed." <<std::endl;

  if (meta_def->generic_param().at("zp_dtype") == "uint8") {
    maxval = 255;
    minval = 0;
  } else if (meta_def->generic_param().at("zp_dtype") == "uint16") {
    is_uint16 = true;
    maxval = 65535;
    minval = 0;
  } else if (meta_def->generic_param().at("zp_dtype") == "int8") {
    is_int8 = true;
    maxval = 127;
    minval = -128;
  } else if (meta_def->generic_param().at("zp_dtype") == "int16") {
    is_int16 = true;
    maxval = 32767;
    minval = -32768;
  }

  if (meta_def->generic_param().at("is_qop") == "1")
    is_qop = true;
  else
    is_qop = false;

  zp = stof(meta_def->generic_param().at("in_zero_point"));
  scale = stof(meta_def->generic_param().at("in_scale"));
}

MyCustomOp::~MyCustomOp() {
  // std::cout << " Custom QDQ_OP destructed." <<std::endl;
}

// Convert float to T with rounding
template <typename T> inline T rounder(float data) {
  static const int data_max = std::numeric_limits<T>::max();
  static const int data_min = std::numeric_limits<T>::min();
  T rlt = 0;
  if (data > data_max) {
    rlt = data_max;
  } else if (data < data_min) {
    rlt = data_min;
  } else if ((data - floor(data)) == 0.5) {
    rlt = std::round(data * 0.5f) * 2.0f;
  } else {
    rlt = static_cast<T>(round(data));
  }
  return rlt;
}

template <typename OutputType>
inline void qlinear_op(const float* src, OutputType* dst,
                       std::size_t num_elements, const float scale,
                       const int zero_point, int minVal, int maxVal) {
  constexpr std::size_t VECTOR_SIZE_BYTES = sizeof(__m512);
  constexpr std::size_t FLOAT_SIZE_BYTES = sizeof(float);
  constexpr std::size_t FLOATS_PER_VECTOR =
      VECTOR_SIZE_BYTES / FLOAT_SIZE_BYTES;

  static_assert(FLOAT_SIZE_BYTES == 4, "Unexpected float size!");

  const std::size_t num_iter = num_elements / FLOATS_PER_VECTOR;
  const std::size_t remainder = num_elements - (num_iter * FLOATS_PER_VECTOR);

  const __m512 scale_vector = _mm512_set1_ps(scale);
  // const __m512 round_vector = _mm512_set1_ps(0.5f);
  const __m512i zero_point_vector = _mm512_set1_epi32(zero_point);

  // to check
  // cout << string(typeid(OutputType).name()) << endl;

  __m512i min_vec = _mm512_set1_epi32(minVal);
  __m512i max_vec = _mm512_set1_epi32(maxVal);

  for (std::size_t i = 0; i < num_iter; ++i) {
    _mm_prefetch((const char*)src + 64, _MM_HINT_T0);
    __m512 in = _mm512_loadu_ps(src);
    in = _mm512_roundscale_ps(_mm512_div_ps(in, scale_vector),
                              _MM_FROUND_TO_NEAREST_INT);
    __m512i in32 = _mm512_add_epi32(_mm512_cvtps_epi32(in), zero_point_vector);
    __m512i clamped =
        _mm512_min_epi32(_mm512_max_epi32(in32, min_vec), max_vec);

    if constexpr (sizeof(OutputType) == 1) {        // int8 / uint8
      _mm_storeu_epi8(dst, _mm512_cvtepi32_epi8(clamped));
    } else if constexpr (sizeof(OutputType) == 2) { // int16 / uint16
      _mm256_storeu_epi16(dst, _mm512_cvtepi32_epi16(clamped));
    }

    src += FLOATS_PER_VECTOR;
    dst += FLOATS_PER_VECTOR;
  }

  if (remainder > 0) {
    // std::transform(src, src + remainder, dst, [&](const float& src) { return
    // rounder<OutputType>((src / scale) + (float)zero_point); });
    std::transform(src, src + remainder, dst, [&](const float& src) {
      float FloatValue;
      FloatValue = std::nearbyintf(src / scale) + (float)zero_point;
      FloatValue = std::max(FloatValue, (float)minVal);
      FloatValue = std::min(FloatValue, (float)maxVal);
      return (OutputType)(int32_t)FloatValue;
    });
  }
}

template <typename InType>
void dqlinear_op(const InType* src, float* dst, std::size_t num_elements,
                 float scale, int zero_point) {
  constexpr std::size_t VECTOR_SIZE_BYTES = sizeof(__m512);
  constexpr std::size_t FLOAT_SIZE_BYTES = sizeof(float);
  constexpr std::size_t FLOATS_PER_VECTOR =
      VECTOR_SIZE_BYTES / FLOAT_SIZE_BYTES;

  static_assert(FLOAT_SIZE_BYTES == 4, "Unexpected float size!");

  const std::size_t num_iter = num_elements / FLOATS_PER_VECTOR;
  const std::size_t remainder = num_elements - (num_iter * FLOATS_PER_VECTOR);

  const __m512 scale_vector = _mm512_set1_ps(scale);
  const __m512i zero_point_vector = _mm512_set1_epi32((int)zero_point);
  for (std::size_t i = 0; i < num_iter; ++i) {
    __m512i in32;
    if constexpr (std::is_same_v<InType, signed char>) {
      _mm_prefetch((const char*)src + 16, _MM_HINT_T0);
      in32 = _mm512_cvtepi8_epi32(_mm_loadu_epi8(src));
    } else if constexpr (std::is_same_v<InType, unsigned char>) {
      _mm_prefetch((const char*)src + 16, _MM_HINT_T0);
      in32 = _mm512_cvtepu8_epi32(_mm_loadu_epi8(src));
    }
    if constexpr (std::is_same_v<InType, short>) {
      _mm_prefetch((const char*)src + 32, _MM_HINT_T0);
      in32 = _mm512_cvtepi16_epi32(_mm256_loadu_epi16(src));
    } else if constexpr (std::is_same_v<InType, unsigned short>) {
      _mm_prefetch((const char*)src + 32, _MM_HINT_T0);
      in32 = _mm512_cvtepu16_epi32(_mm256_loadu_epi16(src));
    }

    __m512 mul = _mm512_mul_ps(
        _mm512_cvtepi32_ps(_mm512_sub_epi32(in32, zero_point_vector)),
        scale_vector);
    _mm512_storeu_ps(dst, mul);
    src += FLOATS_PER_VECTOR;
    dst += FLOATS_PER_VECTOR;
  }

  if (remainder > 0) {
    std::transform(src, src + remainder, dst, [&](const InType& src) {
      return (((int)src - zero_point) * scale);
    });
  }
}

template <typename OutputType>
inline void QuantizeLinear(const float* Input, OutputType* Output, size_t N,
                           float Scale, int ZeroPoint, float MinimumValue,
                           float MaximumValue) {
#if _QDQ_MT_
  auto THREAD_NUM =
      2; // std::max((int)std::thread::hardware_concurrency() / 2, (int)1);
  std::vector<std::future<int>> thr_fut(THREAD_NUM);
  size_t THREAD_WORKLOAD = size_t(ceil((float)N / THREAD_NUM));
  for (size_t i = 0U; i < THREAD_NUM; i++) {
    thr_fut[i] = std::async(
        std::launch::async,
        [&](size_t i) {
          size_t BASE_POS = i * THREAD_WORKLOAD;
          auto workload = std::min(THREAD_WORKLOAD + BASE_POS, N);
          /*float FloatValue;
          for (size_t n = BASE_POS; n < workload; n++) {
            FloatValue = std::nearbyintf(Input[n] / Scale) + ZeroPoint;
            FloatValue = std::max(FloatValue, MinimumValue);
            FloatValue = std::min(FloatValue, MaximumValue);
            Output[n] = (OutputType)(int32_t)FloatValue;
          }*/

          if (workload > BASE_POS)
            qlinear_op((Input + BASE_POS), (Output + BASE_POS),
                       (workload - BASE_POS), Scale, (int)ZeroPoint,
                       (int)MinimumValue, (int)MaximumValue);

          return 0;
        },
        i);
  }

  for (auto i = 0U; i < THREAD_NUM; i++) {
    thr_fut[i].wait();
  }
#else
  qlinear_op(Input, Output, N, Scale, (int)ZeroPoint, (int)MinimumValue,
             (int)MaximumValue);
#endif
}

template <typename InType>
inline void DequantizeLinear(const InType* Input, float* Output, std::size_t N,
                             float Scale, int ZeroPoint) {
#if _QDQ_MT_
  auto THREAD_NUM =
      2; // std::max((int)std::thread::hardware_concurrency() / 2, (int)1);
  std::vector<std::future<int>> thr_fut(THREAD_NUM);
  size_t THREAD_WORKLOAD = size_t(ceil((float)N / THREAD_NUM));
  for (size_t i = 0U; i < THREAD_NUM; i++) {
    thr_fut[i] = std::async(
        std::launch::async,
        [&](size_t i) {
          size_t BASE_POS = i * THREAD_WORKLOAD;
          auto workload = std::min(THREAD_WORKLOAD + BASE_POS, N);
          if (workload > BASE_POS)
            dqlinear_op((Input + BASE_POS), (Output + BASE_POS),
                        (workload - BASE_POS), Scale, (int)ZeroPoint);

          return 0;
        },
        i);
  }

  for (auto i = 0U; i < THREAD_NUM; i++) {
    thr_fut[i].wait();
  }
#else
  dqlinear_op(Input, Output, N, Scale, (int)ZeroPoint);
#endif
}

void MyCustomOp::Compute(const OrtApi* api, OrtKernelContext* context) const {
  // std::cout << " Custom QDQ_OP started." <<std::endl;
  if (Ort::Global<void>::api_ == nullptr) {
    Ort::Global<void>::api_ = api;
  }

  Ort::KernelContext ctx(context);
  auto num_inputs = ctx.GetInputCount();
  auto num_outputs = ctx.GetOutputCount();
  // LOG(INFO) << "num_inputs " << num_inputs << " "   //
  //           << "num_outputs " << num_outputs << " " //
  //     ;
  // std::cout << (int) num_inputs << std::endl;

  auto input_tensor = ctx.GetInput(0);
  // std::cout << "Got input tensor" << std::endl;

  auto tensor_info = input_tensor.GetTensorTypeAndShapeInfo();
  //  std::cout << "Got input tensor info" << std::endl;

  auto element_num = tensor_info.GetElementCount();
  //  std::cout << "Got input tensor" << std::endl;
  // std::cout << (int) element_num << std::endl;

  const float* in_data_f = nullptr;
  const uint16_t* in_data_u16 = nullptr;
  const int16_t* in_data_i16 = nullptr;
  const uint8_t* in_data_u8 = nullptr;
  const int8_t* in_data_i8 = nullptr;

  if (is_qop) {
    auto in_ptr = input_tensor.GetTensorData<float>();
    in_data_f = in_ptr;
  } else if (is_int8) {
    auto in_ptr = input_tensor.GetTensorData<int8_t>();
    in_data_i8 = in_ptr;
  } else if (is_int16) {
    auto in_ptr = input_tensor.GetTensorData<int16_t>();
    in_data_i16 = in_ptr;
  } else if (is_uint16) {
    auto in_ptr = input_tensor.GetTensorData<uint16_t>();
    in_data_u16 = in_ptr;
  } else {
    auto in_ptr = input_tensor.GetTensorData<uint8_t>();
    in_data_u8 = in_ptr;
  }

  auto output_tensor = ctx.GetOutput(0, tensor_info.GetShape());

  float* out_data_f = nullptr;
  uint16_t* out_data_u16 = nullptr;
  int16_t* out_data_i16 = nullptr;
  uint8_t* out_data_u8 = nullptr;
  int8_t* out_data_i8 = nullptr;

  if (is_qop == false)
    out_data_f = output_tensor.GetTensorMutableData<float>();
  else if (is_int8)
    out_data_i8 = output_tensor.GetTensorMutableData<int8_t>();
  else if (is_int16)
    out_data_i16 = output_tensor.GetTensorMutableData<int16_t>();
  else if (is_uint16)
    out_data_u16 = output_tensor.GetTensorMutableData<uint16_t>();
  else
    out_data_u8 = output_tensor.GetTensorMutableData<uint8_t>();

  if (is_qop) {
    // printf ("Quant size: %d\n",(int)element_num);
    auto t1 = std::chrono::steady_clock::now();
    if (is_int8) {
      QuantizeLinear(in_data_f, out_data_i8, element_num, scale, (int)zp,
                     (int)minval, (int)maxval);
    } else if (is_int16) {
      QuantizeLinear(in_data_f, out_data_i16, element_num, scale, (int)zp,
                     (int)minval, (int)maxval);
    } else if (is_uint16) {
      QuantizeLinear(in_data_f, out_data_u16, element_num, scale, (int)zp,
                     (int)minval, (int)maxval);
    } else {
      QuantizeLinear(in_data_f, out_data_u8, element_num, scale, (int)zp,
                     (int)minval, (int)maxval);
    }
    // auto t2 = std::chrono::steady_clock::now();
    // auto rt_ = std::chrono::duration<float, std::micro>(t2 - t1).count();
    // std::cout <<"runtime: " <<  rt_ << std::endl;
  } else {
    if (is_int8) {
      DequantizeLinear(in_data_i8, out_data_f, element_num, scale, (int)zp);
      /*for (int i = 0; i < element_num; i++) {
        float temp = static_cast<float>(in_data_i8[i]);
        temp = (temp - zp) * scale;
        out_data_f[i] = temp;
      }*/
    } else if (is_int16) {
      DequantizeLinear(in_data_i16, out_data_f, element_num, scale, (int)zp);
      /*for (int i = 0; i < element_num; i++) {
        float temp = static_cast<float>(in_data_i16[i]);
        temp = (temp - zp) * scale;
        out_data_f[i] = temp;
      }*/
    } else if (is_uint16) {
      DequantizeLinear(in_data_u16, out_data_f, element_num, scale, (int)zp);
      /*for (int i = 0; i < element_num; i++) {
        float temp = static_cast<float>(in_data_u16[i]);
        temp = (temp - zp) * scale;
        out_data_f[i] = temp;
      }*/
    } else {
      DequantizeLinear(in_data_u8, out_data_f, element_num, scale, (int)zp);
      /*for (int i = 0; i < element_num; i++) {
        float temp = static_cast<float>(in_data_u8[i]);
        temp = (temp - zp) * scale;
        out_data_f[i] = temp;

      }*/
    }
  }
  // std::cout << " Custom QDQ_OP ended." <<std::endl;
}
} // namespace vaip_qdq_op_custom_op
