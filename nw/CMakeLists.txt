cmake_minimum_required(VERSION 3.16)
project(Ethernet8023_Test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # 禁用编译器特有扩展
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT MSVC)
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${SOURCE_DIR})

add_library(EthernetCore STATIC
    ${SOURCE_DIR}/core/frame_type.cc
    ${SOURCE_DIR}/core/utils.cc
    ${SOURCE_DIR}/core/crc_engine.cc
    ${SOURCE_DIR}/core/frame_builder.cc
)

add_executable(test_runner 
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_runner.cc
)

target_link_libraries(test_runner PRIVATE EthernetCore)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/test_cases.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR})