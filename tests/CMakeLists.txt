cmake_minimum_required(VERSION 2.8)
project(clsc_tests)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

add_executable(${PROJECT_NAME}
    main.cpp
    clear_tests.cpp
)

target_link_libraries(${PROJECT_NAME} clsc_utils gtest gmock pthread)
