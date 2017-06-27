cmake_minimum_required(VERSION 3.7)
project(MultiPathProcessing)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp MultiPathProcessing.cpp)
add_executable(MultiPathProcessing ${SOURCE_FILES})