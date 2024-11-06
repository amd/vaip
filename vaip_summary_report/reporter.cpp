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

#include "reporter.hpp"

// Initialize the static unordered_map
std::unordered_map<std::string, std::string> ReportInventory::dataMap;

// ReportInventory instance method
ReportInventory& ReportInventory::getInstance() {
  static ReportInventory instance; // Guaranteed to be created only once
  return instance;
}

// Add data to the map
void ReportInventory::addData(const std::string& key,
                              const std::string& value) {
  dataMap[key] = value;
}

// Get data from the map
std::string ReportInventory::getData(const std::string& key) {
  if (dataMap.find(key) != dataMap.end()) {
    return dataMap[key];
  }
  return "none"; // Default value if the key doesn't exist
}

// Print all the data in the map
void ReportInventory::printData() {
  std::cout << std::endl << "Unsupported op summary:" << std::endl << std::endl;
  for (const auto& pair : dataMap) {
    std::string str = pair.second;
    std::string::size_type pos = 0;
    // Replace underscores with commas
    while ((pos = str.find('_', pos)) != std::string::npos) {
      str.replace(pos, 1, ",");
      ++pos; // Move past the replaced comma
    }
    std::cout << pair.first << ": " << str << std::endl;
    // std::cout << pair.first << ": " << pair.second << std::endl;
  }
}
