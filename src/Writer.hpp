#include <fstream>
#include <exception>
#include <string>
#include <deque>


namespace GDSTXT {
namespace IO {

class Writer {
public:
  Writer(const std::string& filename)
  {
    _file_stream.open(filename, std::ios::binary);
    if (!_file_stream) {
      throw std::runtime_error("failed to open " + filename);
    }
  }

  inline void write(const std::deque<unsigned char>& data);

  ~Writer()
  {
    _file_stream.close();
  }

private:
  std::ofstream _file_stream;
};


}
}

namespace GDSTXT {
namespace IO {

inline
void Writer::write(const std::deque<unsigned char>& data)
{
  if (data.size() > 0) {
    _file_stream.write(reinterpret_cast<const char*>(&(data[0])), data.size());
  }
}


}
}