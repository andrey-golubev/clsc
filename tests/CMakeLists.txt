cmake_minimum_required(VERSION 2.8)
project(clsc_tests)

file(GLOB
    TESTS_SRCS
    *.cpp
    common.hpp
)

add_executable(${PROJECT_NAME} ${TESTS_SRCS})

target_link_libraries(${PROJECT_NAME}
    clsc_utils
    clsc_core
    gtest gmock pthread
)
