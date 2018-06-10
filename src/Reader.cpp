#include "Reader.hpp"
#include <exception>
#include <iostream>

namespace GDSTXT {
namespace IO {

Reader::Reader(const std::string& filename, const FileType filetype)
  : _file_type(filetype)
{
  if (_file_type == FileType::gds) {
    _file_stream = std::ifstream(filename, std::ios::binary);
  } else if (_file_type == FileType::txt) {
    _file_stream = std::ifstream(filename);
  }
  if (! _file_stream)
    throw std::runtime_error("Failed to open " + filename);
}

std::deque<unsigned char> Reader::readStream()
{
  if (this->is_read_done())
    return std::move(std::deque<unsigned char>());

  std::deque<unsigned char> meta_data(4);
  _file_stream.read(reinterpret_cast<char*>(&meta_data[0]), 4);

  auto body_data_size = 
    chars_to_bit_array(std::begin(meta_data), std::begin(meta_data)+2).at(0) - 4;

  std::deque<unsigned char> data(body_data_size + 2);
  data[0] = meta_data[2];
  data[1] = meta_data[3];
  _file_stream.read(reinterpret_cast<char*>(&data[2]),body_data_size);
  return std::move(data);
}


}
}