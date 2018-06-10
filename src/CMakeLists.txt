add_library(Converter convert_func.cpp)

add_library(Reader Reader.cpp)
target_link_libraries(Reader Converter)

add_library(Writer Writer.cpp)
target_link_libraries(Writer Converter)

add_library(Record Record.cpp)
target_link_libraries(Record Converter)
