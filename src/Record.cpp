#include "Record.hpp"
#include "test_config.h"

namespace GDSTXT {

StreamRecord::StreamRecord(const std::deque<unsigned char>& stream_data):
  _bytes_data(
    std::make_shared<std::deque<unsigned char>>(
      std::move(stream_data)))
{
  _tag_name = (*_bytes_data)[0];
  _tag_data_type = (*_bytes_data)[1];
}

StreamRecord::StreamRecord(StreamRecord&& other)
noexcept
{
  _bytes_data = other._bytes_data;
  other._bytes_data = nullptr;
}

StreamRecord& StreamRecord::operator=(StreamRecord&& other)
noexcept
{
  _bytes_data = other._bytes_data;
  other._bytes_data = nullptr;
  return *this;
}

inline
std::string StreamRecord::to_text() const
{
  SPEC::TagDataType tag_data_type = 
    static_cast<SPEC::TagDataType>(_tag_data_type);

  std::vector<std::string> data_str_vec;
  auto data_to_str_func = [&data_str_vec](auto& data) {
    std::for_each(data.begin(), data.end(), [&data_str_vec](auto& i){
      if (typeid(i) == typeid(double)) {
        std::ostringstream os;
        os << i;
        data_str_vec.emplace_back(os.str());
      } else {
        data_str_vec.emplace_back(std::to_string(i));
      }
    });
  };

  switch(tag_data_type) {
    case SPEC::TagDataType::NODATA: {
      break;
    }
    case SPEC::TagDataType::BITARRAY: {
      auto data = chars_to_bit_array(_bytes_data->begin()+2, _bytes_data->end());
      data_to_str_func(data);
      break;
    }
    case SPEC::TagDataType::INTEGER_2: {
      auto data = chars_to_int2(_bytes_data->begin()+2, _bytes_data->end());
      data_to_str_func(data);
      break;
    }
    case SPEC::TagDataType::INTEGER_4: {
      auto data = chars_to_int4(_bytes_data->begin()+2, _bytes_data->end());
      data_to_str_func(data);
      break;
    }
    case SPEC::TagDataType::REAL_4: {
      throw std::runtime_error("real4 is not supported");
      break;
    }
    case SPEC::TagDataType::REAL_8: {
      auto data = chars_to_real8(_bytes_data->begin()+2, _bytes_data->end());
      data_to_str_func(data);
      break;
    }
    case SPEC::TagDataType::ASCII: {
      auto data = chars_to_string(_bytes_data->begin()+2, _bytes_data->end());
      data_str_vec.emplace_back(data);
      break;
    }
    case SPEC::TagDataType::BAD: {
      throw std::runtime_error("bad data type");
      break;
    }
    default: {
      throw std::runtime_error("unknow data type");
      break;
    }
  }

  std::string ret_str = std::get<0>(SPEC::tagname_map.at(_tag_name));
  if (!data_str_vec.empty()) {
    ret_str.append(":");
    for (const auto& i: data_str_vec) {
      ret_str.append(i + " ");
    }
    ret_str.erase(ret_str.length() - 1);
  }

  return std::move(ret_str); 
}

inline
const std::deque<unsigned char> StreamRecord::to_stream() const
{
  return *_bytes_data;
}


TEST_CASE("testing StreamRecord") {
  SUBCASE("test INTEGER_2") {
    std::deque<unsigned char> data {
      0x00, 0x02, 0x00, 0x05
    };
    StreamRecord* ptr = new StreamRecord(data);
    CHECK(ptr->to_text() == "HEADER:5");
    delete ptr;
  }
  SUBCASE("test REAL8") {
    std::deque<unsigned char> data {
      0x03, 0x05, 0x41, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    StreamRecord* ptr = new StreamRecord(data);
    CHECK(ptr->to_text() == "UNITS:1");
    delete ptr;
  }
}

///////////////////////////

AsciiRecord::AsciiRecord(const std::string& string_data)
  :_str_data(string_data)
{
  if (
    std::count(std::begin(string_data), std::end(string_data), '\n') == 1
  ) {
    throw std::runtime_error("one record a time");
  }
}

AsciiRecord::AsciiRecord(AsciiRecord&& other) noexcept
  : _str_data(std::move(other._str_data))
{}

AsciiRecord& AsciiRecord::operator=(AsciiRecord&& other) noexcept
{
  _str_data = std::move(other._str_data);
  return *this;
}

inline
std::string AsciiRecord::to_text() const
{
  return _str_data;
}

inline
const std::deque<unsigned char> AsciiRecord::to_stream() const {
  auto first_col_pos = _str_data.find_first_of(':');
  auto tagname = _str_data.substr(0, first_col_pos);

  while (*(tagname.begin()) == ' ') {
    tagname.erase(tagname.begin());
  }
  while (*(tagname.end()) == ' ') {
    tagname.erase(tagname.end());
  }
  for (auto& i : tagname) toupper(i);

  auto find_iter = std::find_if(SPEC::tagname_map.begin(), SPEC::tagname_map.end(), [&tagname](auto& i){
    return std::get<0>(i.second) == tagname;
  });

  if (find_iter == SPEC::tagname_map.end())
    throw std::runtime_error("unkonw tag name" + tagname);

  std::string data_body;
  if (first_col_pos != std::string::npos) {
    data_body = _str_data.substr(first_col_pos+1);
  } else {
    data_body = "";
  }

  auto data_tag = (*find_iter).first;
  auto data_type = std::get<1>((*find_iter).second);
  std::deque<unsigned char> ret_data;
  switch(static_cast<SPEC::TagDataType>(data_type)) {
    case SPEC::TagDataType::NODATA: {
      ret_data.assign({0x00, 0x04, data_tag, data_type});
      break;
    }
    case SPEC::TagDataType::BITARRAY: {
      ret_data = ascii_to_bit_array(data_body, data_tag, data_type);
      break;
    }
    case SPEC::TagDataType::INTEGER_2: {
      ret_data = ascii_to_int2(data_body, data_tag, data_type);
      break;
    }
    case SPEC::TagDataType::INTEGER_4: {
      ret_data = ascii_to_int4(data_body, data_tag, data_type);
      break;
    }
    case SPEC::TagDataType::REAL_4: {
      std::runtime_error("REAL4 data shouldn't be used");
      break;
    }
    case SPEC::TagDataType::REAL_8: {
      ret_data = ascii_to_real8(data_body, data_tag, data_type);
      break;
    }
    case SPEC::TagDataType::ASCII: {
      ret_data = ascii_to_ascii(data_body, data_tag, data_type);
      break;
    }
    case SPEC::TagDataType::BAD: {
      std::runtime_error("bad data type");
      break;
    }
    default: {
      std::runtime_error("unknow data type");
    }
  }
  return std::move(ret_data);
}


}