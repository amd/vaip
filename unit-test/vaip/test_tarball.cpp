/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
#define _CRT_SECURE_NO_WARNINGS
#include "../vaip/include/vaip/vaip.hpp"
#include "../vaip/src/pass_context_imp.hpp"
#include "../vaip/src/tar_ball.hpp"
#include "debug_logger.hpp"
#include <ctime>
#include <gtest/gtest.h>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>

using namespace vaip_core;
class StringStreamReader : public IStreamReader {
public:
  StringStreamReader(std::stringstream& data) : data_(data) {}

private:
  size_t read(char* data, size_t size) override final {
    return data_.read(data, size).gcount();
  }

private:
  std::stringstream& data_;
};

class StringStreamWriter : public IStreamWriter {

public:
  StringStreamWriter(std::stringstream& data) : data_(data) {}
  size_t write(const char* data, size_t size) override {
    data_.write(data, size);
    return size;
  }

private:
  std::stringstream& data_;
};

class StringStreamWriteBulder : public IStreamWriterBuilder {
public:
  StringStreamWriteBulder(std::map<std::string, std::stringstream>* ss_map)
      : ss_map_(ss_map) {}
  std::unique_ptr<IStreamWriter> build(const std::string& name) override {
    if (ss_map_->count(name) == 0) {
      (*ss_map_)[name] = std::stringstream();
      return std::make_unique<StringStreamWriter>((*ss_map_)[name]);
    }
    return nullptr;
  }

private:
  std::map<std::string, std::stringstream>* ss_map_;
};
class TarBallTest : public DebugLogger {};
TEST_F(TarBallTest, TarTest) {
  std::vector<std::string> test_strings = {"I am ss1", "I am ss2 ,123456",
                                           "I am ss3, 1234567890abcdef"};
  std::map<std::string, std::stringstream> ss_map;
  ss_map["ss0"] = std::stringstream();
  ss_map["ss1"] = std::stringstream();
  ss_map["ss2"] = std::stringstream();
  ss_map["ss0"] << test_strings[0];
  ss_map["ss1"] << test_strings[1];
  ss_map["ss2"] << test_strings[2];
  std::map<std::string, std::stringstream> ss_map_out;

  // 1. create tarball
  std::stringstream tar_sstream;
  {
    TarWriter tar_writer(&StringStreamWriter(tar_sstream));
    for (auto it = ss_map.begin(); it != ss_map.end(); ++it) {
      tar_writer.write(&StringStreamReader(it->second),
                       it->second.str().length(), it->first);
    }
    std::cout << "write tar file finish : " << tar_sstream.str() << std::endl;
  }
  // 2. untar
  {
    StringStreamWriteBulder builder(&ss_map_out);
    TarReader tar_reader(&StringStreamReader(tar_sstream));
    for (;;) {
      bool iscontinue = tar_reader.read(&builder);
      if (!iscontinue) {
        break;
      }
    }
    std::cout << "read tar file finish : " << std::endl;
  }
  bool check_ok = true;
  int i = 0;
  for (auto it = ss_map_out.begin(); it != ss_map_out.end(); ++it) {
    std::cout << it->first << ": " << it->second.str() << std::endl;
    check_ok = check_ok && (it->second.str() == test_strings[i]);
    i++;
  }

  ASSERT_TRUE(check_ok);
}
static std::string generateRandomString(size_t length) {
  const std::string characters = "abcdefghijklmnopqrstuvwxyz"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "0123456789";
  std::random_device rd;
  std::mt19937 generator(rd());
  std::uniform_int_distribution<> distribution(0, characters.size() - 1);
  std::string randomString;
  for (size_t i = 0; i < length; ++i) {
    randomString += characters[distribution(generator)];
  }
  return randomString;
}

TEST_F(TarBallTest, CompressTest) {
  auto data = generateRandomString(65536);
  std::stringstream result;
  {
    std::stringstream data_ss;
    data_ss << data;
    compress(&StringStreamReader(data_ss), &StringStreamWriter(result), 1);
    std::cout << "compress " << data.length()
              << " byes in level=1, result_size = " << result.str().length()
              << std::endl;
  }
  {
    std::stringstream uncompressed_data_ss;

    uncompress(&StringStreamReader(result),
               &StringStreamWriter(uncompressed_data_ss));
    ASSERT_TRUE(data == uncompressed_data_ss.str());
  }

  // test level=default
  {
    result = std::stringstream();
    std::stringstream data_ss;
    data_ss << data;
    compress(&StringStreamReader(data_ss), &StringStreamWriter(result));
    std::cout << "compress " << data.length()
              << " byes in level=9, result_size = " << result.str().length()
              << std::endl;
  }
  {
    std::stringstream uncompressed_data_ss;

    uncompress(&StringStreamReader(result),
               &StringStreamWriter(uncompressed_data_ss));
    ASSERT_TRUE(data == uncompressed_data_ss.str());
  }
}
// todo long filename test