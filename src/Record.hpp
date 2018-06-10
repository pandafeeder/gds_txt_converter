#ifndef __RECORD__H__
#define __RECORD__H__

#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <exception>
#include "convert_func.hpp"
#include "SPEC.hpp"
#include <algorithm>
#include <sstream>

namespace GDSTXT {

class BaseRecord {
  public:
    using BytesData = std::shared_ptr<std::deque<unsigned char>>;
    virtual std::string to_text() const = 0;
    virtual const std::deque<unsigned char> to_stream() const = 0;
    virtual ~BaseRecord() = default;
};

class StreamRecord: public BaseRecord {
  public:
    StreamRecord(const std::deque<unsigned char>& stream_data);
    StreamRecord(const StreamRecord& other) = default;
    StreamRecord& operator=(const StreamRecord& other) = default;
    StreamRecord(StreamRecord&& other) noexcept;
    StreamRecord& operator=(StreamRecord&& other) noexcept;
    virtual std::string to_text() const override;
    virtual const std::deque<unsigned char> to_stream() const override;
    virtual ~StreamRecord() override
    {
      _bytes_data = nullptr;
    }
  private:
    BytesData _bytes_data;
    unsigned char _tag_name;
    unsigned char _tag_data_type;
};


class AsciiRecord: public BaseRecord {
  public:
    AsciiRecord(const std::string& string_data);
    AsciiRecord(const AsciiRecord& other) = default;
    AsciiRecord& operator=(const AsciiRecord& other) = default;
    AsciiRecord(AsciiRecord&& other) noexcept;
    AsciiRecord& operator=(AsciiRecord&& other) noexcept;
    virtual std::string to_text() const override;
    virtual const std::deque<unsigned char> to_stream() const override;
    virtual ~AsciiRecord() override = default;
  private:
    std::string _str_data;
};


}

#endif //__RECORD__H__