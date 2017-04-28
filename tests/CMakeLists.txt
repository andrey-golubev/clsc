cmake_minimum_required(VERSION 2.8)
project(clsc_tests)

add_executable(${PROJECT_NAME}
    main.cpp
    clear_tests.cpp
    ring_array_tests.cpp
    comparable_tests.cpp
)

target_link_libraries(${PROJECT_NAME} clsc_utils gtest gmock pthread)
