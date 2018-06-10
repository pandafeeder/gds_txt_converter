cmake_minimum_required(VERSION 3.4.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

project(PROJ VERSION 1.0 LANGUAGES CXX)
add_subdirectory(src)
add_executable(gds2txt main.cpp)
target_link_libraries(gds2txt Reader Writer Record)