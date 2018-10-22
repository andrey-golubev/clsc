cmake_minimum_required(VERSION 2.8)
project(clsc)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wall -Werror")

option(WITH_ASAN "Enable ASan build flags" OFF)

if (${WITH_ASAN})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak -fsanitize=undefined")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libasan")
endif (${WITH_ASAN})

# gtest
set(BUILD_GMOCK OFF CACHE BOOL "Exclude gmock from build")
set(INSTALL_GTEST OFF CACHE BOOL "Do not install gtest")
add_subdirectory(external/googletest)

# clsc
add_subdirectory(tests)
add_subdirectory(utilities)
