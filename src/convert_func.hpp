#ifndef __CONVERTER__FUNC__H__
#define __CONVERTER__FUNC__H__

#include <vector>
#include <sstream>
#include <iterator>
#include <cstdint>
#include <utility>
#include <bitset>
#include <exception>
#include <string>
#include <algorithm>
#include <cmath>
#include <deque>
#include "test_config.h"

namespace GDSTXT {

using dataIter = std::deque<unsigned char>::iterator;

std::vector<uint16_t> chars_to_bit_array(dataIter start, dataIter end);
std::vector<int16_t>  chars_to_int2(dataIter start, dataIter end);
std::vector<int32_t>  chars_to_int4(dataIter start, dataIter end);
std::vector<double>   chars_to_real8(dataIter start, dataIter end);
std::string chars_to_string(dataIter start, dataIter end);
double _to_real8(dataIter start, dataIter end);

inline
void _check_data(dataIter start, dataIter end, u_int8_t size, const std::string& str)
{
  if (std::distance(start, end) % size != 0)
    throw std::runtime_error(str);
}


template<typename T, std::size_t SIZE>
inline
std::vector<T> spin_data(dataIter start, dataIter end)
{
  union {
    unsigned char ch[SIZE];
    T data;
  } wheel;

  std::vector<T> ret_data;

  while (start != end) {
    for (std::size_t i = 0; i < SIZE; ++i) {
        wheel.ch[i] = *(start++);
    }
    //gds is big-endian, reverse it for most CPU nowadays are little-endia
    std::reverse(std::begin(wheel.ch), std::end(wheel.ch));
    ret_data.push_back(wheel.data);
  }
  return std::move(ret_data);
}

inline
std::vector<std::string> str_data_block(const std::string& str)
{
  std::istringstream in_str_stream (str);
  std::vector<std::string> str_data_vec;
  std::string str_data;
  while (in_str_stream >> str_data) {
    str_data_vec.push_back(str_data);
  }
  return std::move(str_data_vec);
}

// converter for string to bit_array, int2, int4
template<std::size_t N, unsigned char C>
inline
std::deque<unsigned char> suck_data(std::string data_str)
{
  std::deque<unsigned char> ret_data;
  std::string bits_str;
  switch (N) {
    case 16: {
      switch (C) {
        case 0x01: {
          bits_str = std::bitset<16>(std::stoul(data_str, nullptr, 10)).to_string();
          break;
        }
        case 0x02: {
          bits_str = std::bitset<16>(std::stol(data_str, nullptr, 10)).to_string();
          break;
        }
        default: {
          throw std::runtime_error("2 bytes data only include bit_array and int2");
        }
      }
      break;
    }
    case 32: {
      bits_str = std::bitset<32>(std::stol(data_str, nullptr, 10)).to_string();
      break;
    }
    default: {
        throw std::runtime_error("only real8 has more than 4 bytes");
    }
  };
  for (int i = 0; i < N/8; ++i) {
    ret_data.push_back(
      static_cast<unsigned char>(std::bitset<8>(bits_str,i*8, 8).to_ulong())
    );
  }
  return std::move(ret_data);
}


// record_meta_data means first 4 bytes of record consisting:
// record data size, record name, record data type
template<std::size_t N>
inline
void push_record_meta_data(
  std::vector<std::string>& str_data_vec,
  std::deque<unsigned char>& data,
  unsigned char tagname,
  unsigned char tag_data_type
)
{
  auto data_size = str_data_vec.size()*(N/8) + 4;
  std::bitset<16> bit_size (data_size);
  auto bit_str = bit_size.to_string();
  for (int i = 0; i < 2; ++i) {
    data.push_back(static_cast<unsigned char>(std::bitset<8>(bit_str,i*8, 8).to_ulong()));
  }
  data.push_back(tagname);
  data.push_back(tag_data_type);
}

template<std::size_t N, unsigned char C>
inline
void
push_record_body_data(std::vector<std::string>& str_data_vec, std::deque<unsigned char>& ret_data)
{
  for (const auto& i : str_data_vec) {
    for (const auto& j : suck_data<N, C>(i)) {
      ret_data.push_back(j);
    }
  }
}

std::deque<unsigned char>
ascii_to_bit_array(const std::string& str, unsigned char tagname, unsigned char tag_data_type);

std::deque<unsigned char>
ascii_to_int2(const std::string& str, unsigned char tagname, unsigned char tag_data_type);

std::deque<unsigned char>
ascii_to_int4(const std::string& str, unsigned char tagname, unsigned char tag_data_type);

std::deque<unsigned char>
ascii_to_real8(const std::string& str, unsigned char tagname, unsigned char tag_data_type);

std::deque<unsigned char>
ascii_to_ascii(std::string str, unsigned char tagname, unsigned char tag_data_type);


}



#endif //__CONVERTER__FUNC__H__