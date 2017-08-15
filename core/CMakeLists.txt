cmake_minimum_required(VERSION 2.8)
project(clsc_core)

file(GLOB
    CORE_SRCS
    *.hpp
)

add_library(${PROJECT_NAME} STATIC
    ${CORE_SRCS}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
