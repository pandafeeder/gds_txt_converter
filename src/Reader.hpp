#ifndef __READER__H__
#define __READER__H__

#include <string>
#include <utility>
#include <deque>
#include <fstream>
#include <exception>
#include <iterator>
#include "convert_func.hpp"

namespace GDSTXT {
namespace IO {

class Reader {
  public:
    enum class FileType {
      gds,
      txt
    };
    Reader(const std::string& filename, const FileType filetype);
    Reader(const Reader&) = delete;
    Reader& operator=(const Reader&) = delete;
    std::deque<unsigned char> readStream();
    inline std::string readText();
    inline bool is_read_done() noexcept;
    ~Reader()
    {
      _file_stream.close();
    }
  private:
    std::ifstream _file_stream;
    FileType _file_type;
};

}
}

namespace GDSTXT {
namespace IO {

inline
std::string Reader::readText()
{
  if (this->is_read_done())
    return "";

  std::string line;
  std::getline(_file_stream, line);
  return std::move(line);
}

inline
bool Reader::is_read_done() noexcept
{
  return _file_stream.peek() == std::ifstream::traits_type::eof();
}


}
}

#endif //__READER__H__