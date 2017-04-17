cmake_minimum_required(VERSION 2.8)
project(clsc_tests)

set(HDRS_DIR "../utilities")

add_executable(${PROJECT_NAME}
    ${HDRS_DIR}/clear.hpp
    ${HDRS_DIR}/comparable.hpp

    main.cpp
    clear_tests.cpp
    comparable_tests.cpp
)

target_link_libraries(${PROJECT_NAME} gtest gmock pthread)
