cmake_minimum_required(VERSION 2.8)
project(clsc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(interfaces utilities)

add_subdirectory(tests)
