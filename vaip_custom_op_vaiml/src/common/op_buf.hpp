/*
 *     The Xilinx Vitis AI Vaip in this distribution are provided under the
 * following free and permissive binary-only license, but are not provided in
 * source code form.  While the following free and permissive license is similar
 * to the BSD open source license, it is NOT the BSD open source license nor
 * other OSI-approved open source license.
 *
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
#ifndef __OP_BUF_HPP__
#define __OP_BUF_HPP__

#include "op_init.hpp"
#include <vector>

// namespace vaip_vaiml_custom_op {
class op_buf {

public:
  op_buf() {}
  ~op_buf() {}

  void addOP(const instr_base& instr) {
    unsigned ibuf_sz = (unsigned)(ibuf_.size());
    // std::cout << "OP TYPE: " << instr.type() << " instr size: " <<
    // instr.size() << " ibuf size: " << ibuf_.size() << std::endl;
    ibuf_.resize(ibuf_sz + instr.size());
    instr.serialize((void*)&ibuf_[ibuf_sz]);
    // memcpy ( &ibuf_[ibuf_sz], op_ptr, op_ptr->size_in_bytes);
    // std::cout << "ibuf size: " << ibuf_.size() << std::endl;
  }

  size_t size() const { return ibuf_.size(); }

  const void* data() const { return ibuf_.data(); }

  void clear() { ibuf_.clear(); }

private:
  std::vector<uint8_t> ibuf_;
};

// } // namespace
#endif