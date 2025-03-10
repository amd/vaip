/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */

#include <vector>

using dims_t = std::vector<int>;

enum PRINT_LEVEL : char { ALL = 0, TYPE_SHAPE = 1, PARTIAL_DATA = 2 };

template <class dtype> class WTensor {
public:
  WTensor() {
    _data.clear();
    _shape.clear();
    _strides.clear();
  }
  WTensor(const dims_t& dim, const dtype init_val = 0) {
    _shape = dim;
    _data.resize(prod(dim), init_val);
    calcStrides();
  }
  WTensor(const WTensor& t) {
    _shape = t._shape;
    calcStrides();
    for (auto& val : t._data) {
      _data.push_back(val);
    }
  }
  WTensor& operator=(const WTensor& t) {
    _shape = t._shape;
    calcStrides();
    _data.clear();
    for (auto& val : t._data) {
      _data.push_back(val);
    }
    return *this;
  }
  WTensor(WTensor&& other) noexcept {
    _shape = std::move(other._shape);
    _data = std::move(other._data);
    _strides = std::move(other._strides);
  }
  dims_t shape() { return _shape; }
  uint32_t size() { return prod(_shape); }
  auto& data() { return _data; }
  WTensor& arange(int rg) {
    _data.clear();
    for (int i = 0; i < rg; i++) {
      _data.push_back(static_cast<dtype>(i));
    }
    _shape.clear();
    _shape.push_back(rg);
    calcStrides();
    return *this;
  }
  WTensor& reshape(const dims_t& dim) {
    if (prod(dim) != size()) {
      std::cerr << "--- WTensor: Error: reshape size mismatch!" << std::endl;
      return *this;
    }
    _shape = dim;
    calcStrides();
    return *this;
  }

  WTensor& print(std::string prefix = "", PRINT_LEVEL level = PARTIAL_DATA) {
    std::cout << prefix;
    std::cout << "WTensor{shape=(";
    for (int i = 0; i < _shape.size(); ++i) {
      std::cout << _shape[i];
      if (i < _shape.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "), dtype=" << typeid(dtype).name() << "}\n    ";
    if (level == ALL) {
      for (int i = 0; i < _data.size(); i++) {
        // std::cout << _data[i] << " ";
        if constexpr (sizeof(dtype) == 8) {
          std::cout << _data[i] << " ";
        } else {
          printf("%d ", _data[i]);
        }
      }
      std::cout << std::endl;
    } else if (level == PARTIAL_DATA) {
      for (int i = 0; i < 4; i++) {
        // std::cout << _data[i] << " ";
        if constexpr (sizeof(dtype) == 8) {
          std::cout << _data[i] << " ";
        } else {
          printf("%d ", _data[i]);
        }
      }
      std::cout << "... ";
      for (int i = (_data.size() - 4); i < _data.size(); i++) {
        // std::cout << _data[i] << " ";
        if constexpr (sizeof(dtype) == 8) {
          std::cout << _data[i] << " ";
        } else {
          printf("%d ", _data[i]);
        }
      }
      std::cout << std::endl;
    }

    return *this;
  }
  dtype operator()(const dims_t& dim) {
    if (dim.size() != _shape.size()) {
      std::cerr << "--- WTensor: Error: operator() index size mismatch!"
                << std::endl;
      return 0;
    }
    uint32_t index = 0;
    for (int i = 0; i < dim.size(); i++) {
      if (dim[i] >= _shape[i]) {
        std::cerr << "--- WTensor: Error: operator() index out of range!"
                  << std::endl;
        return 0;
      }
      index = index * _shape[i] + dim[i];
    }
    return _data[index];
  }
  WTensor slice(const dims_t& start, const dims_t& end) {
    if ((start.size() != end.size()) || (start.size() != _shape.size())) {
      std::cerr << "--- WTensor: Error: slice size mismatch!" << std::endl;
      return *this;
    }
    dims_t newShape;
    for (int i = 0; i < start.size(); i++) {
      int dif = end[i] - start[i];
      if (dif > 0) {
        if (end[i] <= _shape[i]) {
          newShape.push_back(dif);
        } else {
          std::cerr
              << "--- WTensor: Error: slice end dim must less than shape dim."
              << std::endl;
          return *this;
        }
      } else {
        std::cerr
            << "--- WTensor: Error: slice end dim must bigger than start dim."
            << std::endl;
        return *this;
      }
    }

    WTensor newTensor = WTensor(newShape);
    sliceData(_data, newTensor._data, start, end, _shape);
    return newTensor;
  }

  WTensor reduceSum(const int32_t dim) {
    if (dim < 0 || dim >= _shape.size()) {
      std::cerr << "--- WTensor: Error: reduceSum dim out of range!"
                << std::endl;
      return *this;
    }
    dims_t newShape = _shape;
    newShape[dim] = 1;
    WTensor result = WTensor(newShape);
    dims_t start(_shape.size(), 0);
    dims_t end = _shape;
    for (int i = 0; i < _shape[dim]; i++) {
      start[dim] = i;
      end[dim] = i + 1;
      auto tmp = slice(start, end);
      for (int n = 0; n < tmp.size(); n++) {
        result._data[n] += tmp._data[n];
      }
    }

    return result;
  }
  WTensor mul(const dtype val) {
    WTensor result = WTensor(_shape);
    for (int i = 0; i < _data.size(); i++) {
      result._data[i] = _data[i] * val;
    }
    return result;
  }
  WTensor add(const dtype val) {
    WTensor result = WTensor(_shape);
    for (int i = 0; i < _data.size(); i++) {
      result._data[i] = _data[i] + val;
    }
    return result;
  }
  WTensor sub(const dtype val) {
    WTensor result = WTensor(_shape);
    for (int i = 0; i < _data.size(); i++) {
      result._data[i] = _data[i] - val;
    }
    return result;
  }
  WTensor add(const WTensor& t) {
    if (_shape != t._shape) {
      std::cerr << "--- WTensor: Error: add shape mismatch!" << std::endl;
      return *this;
    }
    WTensor result = WTensor(_shape);
    for (int i = 0; i < _data.size(); i++) {
      result._data[i] = _data[i] + t._data[i];
    }
    return result;
  }
  WTensor transpose(const dims_t& perm) {
    if (perm.size() != _shape.size()) {
      std::cerr << "--- WTensor: Error: transpose perm size mismatch!"
                << std::endl;
      return *this;
    }
    WTensor result = WTensor(_shape);
    dims_t newShape;
    for (int i = 0; i < perm.size(); i++) {
      newShape.push_back(_shape[perm[i]]);
    }
    result.reshape(newShape);
    for (int i = 0; i < result.size(); i++) {
      dims_t srcIdx = getDimFromIdx(i);
      dims_t dstIdx;
      for (int j = 0; j < perm.size(); j++) {
        dstIdx.push_back(srcIdx[perm[j]]);
      }
      result._data[result.getAbsIndex(dstIdx)] = _data[i];
    }

    return result;
  }
  WTensor concat(const WTensor& tensor1d) {
    // currently only support concat 1D tensor
    if (_shape.size() != 1 || tensor1d._shape.size() != 1) {
      std::cerr << "--- WTensor: Error: concat only support 1D tensor!"
                << std::endl;
      return *this;
    }
    WTensor result = WTensor::createFromVector(_data);
    result._data.insert(result._data.end(), tensor1d._data.begin(),
                        tensor1d._data.end());
    result._shape[0] = _shape[0] + tensor1d._shape[0];
    result.calcStrides();

    return result;
  }
  WTensor reorder(const dims_t& order, const int aixs = 0) {
    // Currently only support reorder on axis = 0
    if (aixs != 0) {
      // std::cerr << "--- WTensor: Error: reorder only support axis = 0!"
      //           << std::endl;
      return *this;
    }
    if (order.size() != _shape[0]) {
      // std::cerr << "--- WTensor: Error: reorder order size mismatch!"
      //           << std::endl;
      // std::cout << "order size: " << order.size() << ", shape[0]: " <<
      // _shape[0]
      //           << std::endl;
      // printf("order: %d, %d, %d, %d\n", order[0], order[1], order[2],
      // order[3]);
      exit(1);
      return *this;
    }
    dims_t tmpShape = _shape;
    tmpShape[0] = 1;
    dims_t start = dims_t(_shape.size(), 0);
    dims_t end = _shape;
    WTensor result;
    result._shape.push_back(0);
    for (int i = 0; i < _shape[0]; i++) {
      start[0] = order[i];
      end[0] = start[0] + 1;
      auto tmp = slice(start, end).reshape({prod(tmpShape)});
      result = result.concat(tmp);
    }
    result.reshape(_shape);
    return result;
  }
  WTensor pad(const std::vector<std::vector<int>> pads, const int pad_val = 0) {
    if (pads.size() != _shape.size()) {
      std::cerr << "--- WTensor: Error: pad pads size mismatch!" << std::endl;
      return *this;
    }
    dims_t newShape;
    for (int i = 0; i < pads.size(); i++) {
      newShape.push_back(_shape[i] + pads[i][0] + pads[i][1]);
    }
    WTensor result = WTensor(newShape, pad_val);
    for (int i = 0; i < _data.size(); i++) {
      dims_t srcDim = getDimFromIdx(i);
      dims_t dstDim = srcDim;
      for (int k = 0; k < pads.size(); k++) {
        dstDim[k] += pads[k][0];
      }
      int dstIdx = result.getAbsIndex(dstDim);
      result._data[dstIdx] = _data[i];
    }
    return result;
  }
  static WTensor createFromVector(const std::vector<int8_t>& srcV) {
    WTensor result;
    result._shape.push_back(srcV.size());
    for (auto& val : srcV) {
      result._data.push_back(static_cast<dtype>(val));
    }
    result.calcStrides();
    return result;
  }
  static WTensor createFromVector(const std::vector<uint8_t>& srcV) {
    WTensor result;
    result._shape.push_back(srcV.size());
    for (auto& val : srcV) {
      result._data.push_back(static_cast<dtype>(val));
    }
    result.calcStrides();
    return result;
  }
  static WTensor createFromVector(const std::vector<int16_t>& srcV) {
    WTensor result;
    result._shape.push_back(srcV.size());
    for (auto& val : srcV) {
      result._data.push_back(static_cast<dtype>(val));
    }
    result.calcStrides();
    return result;
  }
  static WTensor createFromVector(const std::vector<uint16_t>& srcV) {
    WTensor result;
    result._shape.push_back(srcV.size());
    for (auto& val : srcV) {
      result._data.push_back(static_cast<dtype>(val));
    }
    result.calcStrides();
    return result;
  }
  static WTensor createFromVector(const std::vector<int32_t>& srcV) {
    WTensor result;
    result._shape.push_back(srcV.size());
    for (auto& val : srcV) {
      result._data.push_back(static_cast<dtype>(val));
    }
    result.calcStrides();
    return result;
  }
  static WTensor createFromVector(const std::vector<double>& srcV) {
    WTensor result;
    result._shape.push_back(srcV.size());
    for (auto& val : srcV) {
      result._data.push_back(static_cast<dtype>(val));
    }
    result.calcStrides();
    return result;
  }

private:
  int32_t prod(const dims_t& dim) {
    uint32_t res = 1;
    for (const auto& d : dim) {
      res *= d;
    }
    return res;
  }
  void calcStrides() {
    dims_t strides(_shape.size(), 1);
    for (int i = _shape.size() - 2; i >= 0; i--) {
      strides[i] = strides[i + 1] * _shape[i + 1];
    }
    _strides = strides;
  }
  int getAbsIndex(const dims_t& curIdx) {
    int index = 0;
    for (int i = 0; i < curIdx.size(); i++) {
      index += curIdx[i] * _strides[i];
    }
    return index;
  }
  dims_t getDimFromIdx(const int idx) {
    dims_t curIdx(_shape.size(), 0);
    int index = idx;
    for (int i = 0; i < curIdx.size(); i++) {
      curIdx[i] = index / _strides[i];
      index = index % _strides[i];
    }
    return curIdx;
  }
  void sliceData(const std::vector<dtype>& srcData, std::vector<dtype>& dstData,
                 const dims_t& start, const dims_t& end, const dims_t& shape) {
    int srcIndex = 0;
    int dstIndex = 0;
    for (int i = 0; i < shape.size(); i++) {
      srcIndex += start[i] * _strides[i];
    }

    dims_t currIndex = start;
    while (true) {
      dstData[dstIndex++] = srcData[srcIndex++];

      int dim = shape.size() - 1;
      while (dim >= 0 && ++currIndex[dim] >= end[dim]) {
        currIndex[dim] = start[dim];
        dim--;
      }
      srcIndex = getAbsIndex(currIndex);
      if (dim < 0) {
        break;
      }
    }
  }

  dims_t _shape;
  dims_t _strides;
  std::vector<dtype> _data;
};

struct lstm_init_wts {
  double scale[24];
  uint8_t zp[24];
  uint32_t lstm_320_rtp[16];
  uint32_t lstm_1024_rtp[16];
  int8_t* lstm0_h_wts; //[1*4096*1024];
  int8_t* lstm0_x_wts; //[1*4096*320];
  int8_t* lstm0_bias;  //[1*8192];
  int8_t* lstm1_h_wts; //[1*4096*1024];
  int8_t* lstm1_x_wts; //[1*4096*1024];
  int8_t* lstm1_bias;  //[1*8192];
};

class LstmSettings {
public:
  LstmSettings(int layer) : layer_id(layer) {
    // std::cout << "###### LstmSetting: " << layer << " ######" << std::endl;
    // std::cout << std::fixed
    //           <<
    //           std::setprecision(std::numeric_limits<double>::max_digits10);
    //  ---- data shape settings ----
    len_x = 320;  // 64; // dim k
    len_h = 1024; // 128; // dim n
    sv_K = 64;
    sv_N = 64;
    c_sv_idx = 0;
    output_linear = 0;
    n_iter = len_h / (sv_N / 4);
    // wts gen
    len_k = len_x;
    len_kp = 1024;
    len_n = 1024;
    sv_k = 64;
    sv_n0 = 16;
    sv_n = sv_n0 * 4;
    num_row = 4;
    k_iter = len_kp / sv_k;
    n_iter_sv = len_n / sv_n0;
    if (layer == 320) {
      layer_name = "lstm_320";
      len_x = 320; // 64; // dim k
    } else if (layer == 1024) {
      layer_name = "lstm_1024";
      len_x = 1024; // 64; // dim k
    }
  }

  // ------ global settings ------
  int layer_id;
  std::string layer_name;
  std::string data_path;
  // ---- data shape settings ----
  int len_x;
  int len_h;
  int sv_K;
  int sv_N;
  int c_sv_idx;
  int output_linear;
  int n_iter;
  // wts gen
  int len_k;
  int len_kp;
  int len_n;
  int sv_k;
  int sv_n0;
  int sv_n;
  int num_row;
  int k_iter;
  int n_iter_sv;
};