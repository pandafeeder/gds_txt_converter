#include "convert_func.hpp"
#include "test_config.h"
#include <cmath>

namespace GDSTXT {


std::vector<uint16_t> chars_to_bit_array(dataIter start, dataIter end)
{
  _check_data(start, end, 2, "bit array data is corrupted");
  return std::move(spin_data<uint16_t, 2>(start, end));
}

TEST_CASE("testing chars_to_bit_array") {
  SUBCASE("0 char concat 0 char should be 0") {
    std::deque<unsigned char> data {0x00, 0x00};
    auto ret = chars_to_bit_array(data.begin(), data.end());
    CHECK(ret[0] == 0);
  }
  SUBCASE("1 char concat 1 char should be 257") {
    std::deque<unsigned char> data {0x01, 0x01};
    auto ret = chars_to_bit_array(data.begin(), data.end());
    CHECK(ret[0] == 257);
  }
  SUBCASE("should throw when input data size is odd") {
    std::deque<unsigned char> data(3);
    CHECK_THROWS_AS(chars_to_bit_array(data.begin(), data.end()), std::exception);
  }
}

///////////////////////////////////////////

std::vector<int16_t> chars_to_int2(dataIter start, dataIter end)
{
  _check_data(start, end, 2, "int2 data is corrupted");
  return std::move(spin_data<int16_t, 2>(start, end));
}

TEST_CASE("testing chars2_to_int2") {
  SUBCASE("bytes of 1 1 0 1 should be 128 and 1") {
    std::deque<unsigned char> data {0x01, 0x01, 0x00, 0x01};
    auto ret = chars_to_int2(data.begin(), data.end());
    CHECK(ret.size() == 2);
    CHECK(ret[0] == 257);
    CHECK(ret[1] == 1);
  }
}

///////////////////////////////////////////

std::vector<int32_t> chars_to_int4(dataIter start, dataIter end)
{
  _check_data(start, end, 4, "int4 data is corrupted");
  return std::move(spin_data<int32_t, 4>(start, end));
}

TEST_CASE("testing chars2_to_int4") {
  SUBCASE("bytes of 0 0 0 2 should be 2") {
    std::deque<unsigned char> data {0x00, 0x00, 0x00, 0x02};
    auto ret = chars_to_int4(data.begin(), data.end());
    CHECK(ret.size() == 1);
    CHECK(ret[0] == 2);
  }
  SUBCASE("should throw when input data size % 4 != 0") {
    std::deque<unsigned char> data(6);
    CHECK_THROWS_AS(chars_to_int4(data.begin(), data.end()), std::exception);
  }
}

///////////////////////////////////////////

double _to_real8(dataIter start, dataIter end)
{
  std::string bits_string;
  //for (; start != end; ++start) {
  while (start !=end ) {
    bits_string.append(std::bitset<8>(*(start++)).to_string());
  }
  std::bitset<64> bits(bits_string);
  auto mant = (bits & std::bitset<64>(0x00ffffffffffffff)).to_ullong();
  auto bits_shift = bits >> 56;
  auto expr = (bits_shift & std::bitset<64>(0x000000000000007f)).to_ullong();
  double ret = static_cast<double>(ldexp(mant, 4 * (expr -64) - 56));
  if (bits.test(63))
    ret *= -1;
  return ret;
}

std::vector<double> chars_to_real8(dataIter start, dataIter end)
{
  _check_data(start, end, 8, "real8 data is corrupted");
  std::vector<double> ret_data;

  //for (auto iter = data.cbegin(); iter != data.cend(); iter += 8) {
  //  std::deque<unsigned char> data_unit (iter, iter+8);
  //  ret_data.push_back(_to_real8(data_unit));
  //}

  while (start != end) {
    ret_data.push_back(_to_real8(start, start+8));
    start += 8;
  }

  return std::move(ret_data);
}

TEST_CASE("testing char2_to_real8") {
  SUBCASE("should throw when input data size % 8 != 0") {
    std::deque<unsigned char> data(12);
    CHECK_THROWS_AS(chars_to_real8(data.begin(), data.end()), std::exception);
  }
  SUBCASE("bytes 0x4110000000000000 should be 1.0") {
    std::deque<unsigned char> data
    {0x41, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    auto ret = chars_to_real8(data.begin(), data.end());
    CHECK(ret.size() == 1);
    CHECK(ret[0] == 1.0);
  }
  SUBCASE("bytes 0x0000000000000000, 0x4110000000000000, 0xc120000000000000"
   "should return 0.0, 1.0, -2.0") {
    std::deque<unsigned char> data
    {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x41, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      static_cast<unsigned char>(0xc1), 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    auto rec = chars_to_real8(data.begin(), data.end());
    CHECK(rec.size() == 3);
    CHECK(rec[0] == 0.0);
    CHECK(rec[1] == 1.0);
    CHECK(rec[2] == -2.0);
  }
}

///////////////////////////////////////////

std::string chars_to_string(dataIter start, dataIter end)
{
  std::string str (start, end);
  while (str[str.length() -1 ] == '\0') {
    str.erase(str.length() - 1);
  }
  return std::move(str);
}

TEST_CASE("testing char2_to_string") {
  std::deque<unsigned char> data
  {0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x00, 0x00};
  auto ret = chars_to_string(data.begin(), data.end());
  CHECK(ret == "ABCDEF");
}


///////////////////////////////////////////
std::deque<unsigned char>
ascii_to_bit_array(const std::string& str, unsigned char tagname, unsigned char tag_data_type)
{
  auto str_data_vec = str_data_block(str);
  std::deque<unsigned char> ret_data;
  push_record_meta_data<16>(str_data_vec, ret_data, tagname, tag_data_type);
  push_record_body_data<16, 0x01>(str_data_vec, ret_data);
  return ret_data;
}

std::deque<unsigned char>
ascii_to_int2(const std::string& str, unsigned char tagname, unsigned char tag_data_type)
{
  auto str_data_vec = str_data_block(str);
  std::deque<unsigned char> ret_data;
  push_record_meta_data<16>(str_data_vec, ret_data, tagname, tag_data_type);
  push_record_body_data<16, 0x02>(str_data_vec, ret_data);
  return ret_data;
}

std::deque<unsigned char>
ascii_to_int4(const std::string& str, unsigned char tagname, unsigned char tag_data_type)
{
  auto str_data_vec = str_data_block(str);
  std::deque<unsigned char> ret_data;
  push_record_meta_data<32>(str_data_vec, ret_data, tagname, tag_data_type);
  push_record_body_data<32, 0x03>(str_data_vec, ret_data);
  return ret_data;
}


std::deque<unsigned char>
ascii_to_ascii(std::string str, unsigned char tagname, unsigned char tag_data_type)
{
  std::deque<unsigned char> ret_data;

  auto str_length = str.length();
  auto data_size = str_length % 2 == 0 
    ? str_length + 4
    : str_length + 5;

  std::bitset<16> bit_size (data_size);
  auto bit_str = bit_size.to_string();
  for (int i = 0; i < 2; ++i) {
    ret_data.push_back(static_cast<unsigned char>(std::bitset<8>(bit_str,i*8, 8).to_ulong()));
  }
  ret_data.push_back(tagname);
  ret_data.push_back(tag_data_type);

  for (const auto& i : str) {
    ret_data.push_back(i);
  }
  if (str_length % 2 != 0) {
    ret_data.push_back('\0');
  }
  return ret_data;
}

////////////////////////////////////////

inline std::vector<char>
_str_to_real8(const std::string& str)
{
  double value = std::strtod(str.c_str(), nullptr);
  std::vector<char> bytes(8);
  bytes[0] = 0;

  if (value < 0) {
    bytes[0] = 0x80;
    value = -value;
  }

  int e = 0;
  if (value < 1e-77) {
    value = 0;
  } else {
    double lg16 = log(value) / log(16.0);
    e = int (ceil(log(value) / log(16.0)));
    if (e == lg16) {
      ++e;
    }
  }
  value /= pow(16.0, e - 14);
  bytes[0] |= ((e + 64) & 0x7f);
  uint64_t m = uint64_t (value + 0.5);
  for (int i = 7; i > 0; --i) {
    bytes[i] = (m & 0xff);
    m = m >> 8;
  }
  return std::move(bytes);
}

std::deque<unsigned char>
ascii_to_real8(const std::string& str, unsigned char tagname, unsigned char tag_data_type)
{
  auto str_data_vec = str_data_block(str);
  std::deque<unsigned char> ret_data;
  push_record_meta_data<64>(str_data_vec, ret_data, tagname, tag_data_type);

  for (const auto& i : str_data_vec) {
    for (const auto& j : _str_to_real8(i)) {
      ret_data.push_back(j);
    }
  }
  return ret_data;
}

}