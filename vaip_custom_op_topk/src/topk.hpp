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
#pragma once

#include <numeric>

#include "pp_topk_instr_compiler.hpp"
#include "xf_aie_const.hpp"

class TopK {
public:
  TopK(xrt::device& device, xrt::kernel& kernel, std::vector<int64_t>& in_shape,
       std::vector<int64_t>& out_shape) {
    // Input/output sizes
    int64_t in_size = 1;
    in_size = std::accumulate(in_shape.begin(), in_shape.end(), in_size,
                              std::multiplies());
    int64_t out_size = 1;
    // Alignment needed for TOP-K output
    int64_t alignment = 8;
    out_size = ((out_size + (alignment - 1)) / alignment) * alignment;
    out_size = std::accumulate(out_shape.begin(), out_shape.end(), out_size,
                               std::multiplies());

    rtpData = new uint16_t[RTP_SIZE >> 1];

    uint16_t opcode = PP_TOPK;
    rtpData[PP_OPCODE] = opcode;
    rtpData[PP_TOPK_RTP_NUM_ELEM] = static_cast<uint16_t>(in_size);
    rtpData[PP_TOPK_RTP_K] = static_cast<uint16_t>(out_size);
    rtpData[PP_TOPK_RTP_START_IDX] = 0;

    std::cout << "in, out sizes: " << in_size << ", " << out_size << "\n";

    size_t metadata_size = RTP_SIZE; // bytes
    size_t metadata_words = 1 + (metadata_size / sizeof(uint32_t));

    // Create a buffer to hold metadata + instructions
    instr_buffer_topk = new uint32_t[30000];

    // Load metadata
    instr_buffer_topk[0] = static_cast<uint32_t>(metadata_words);
    memcpy(instr_buffer_topk + 1, rtpData, RTP_SIZE);

    // Create instance of compiler
    auto compiler =
        std::make_unique<TopkInstrCompiler>(instr_buffer_topk + metadata_words);
    instr_counter = compiler->generate(rtpData);
    instr_counter = static_cast<uint32_t>(instr_counter + metadata_words);

    // Create BOs
    bo_in =
        xrt::bo(device, in_size, XRT_BO_FLAGS_HOST_ONLY, kernel.group_id(2));
    bo_out = xrt::bo(device, 2 * out_size * sizeof(uint16_t),
                     XRT_BO_FLAGS_HOST_ONLY, kernel.group_id(3));
  }

  ~TopK() {
    if (instr_buffer_topk)
      delete[] instr_buffer_topk;
    if (rtpData)
      delete[] rtpData;
  }

  uint16_t* get_host_buffer_in() { return bo_in.map<uint16_t*>(); }
  uint16_t* get_host_buffer_out() { return bo_out.map<uint16_t*>(); }

  size_t get_instr_size() { return instr_counter * sizeof(uint32_t); }

  void sync_instructions(xrt::bo& bo_instr) {
    memcpy(bo_instr.map<void*>(), instr_buffer_topk,
           instr_counter * sizeof(uint32_t));
    bo_instr.sync(XCL_BO_SYNC_BO_TO_DEVICE, instr_counter * sizeof(uint32_t),
                  0);
  }

  void exec(xrt::kernel& kernel, xrt::bo& bo_instr) {
    // Sync Input BOs
    bo_in.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    // Set kernel argument and trigger it to run
    auto run = kernel(bo_instr, instr_counter, bo_in, bo_out);

    // Wait for kernel to be done
    run.wait();

    // Sync outputs(score and indices) back to host
    bo_out.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
  }

public:
  xrt::bo bo_in, bo_out;
  uint32_t* instr_buffer_topk;
  uint16_t* rtpData;
  uint32_t instr_counter;
};
