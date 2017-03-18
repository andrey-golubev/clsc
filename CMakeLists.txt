cmake_minimum_required(VERSION 2.8)
project(clsc)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(utilities)

add_subdirectory(tests)
