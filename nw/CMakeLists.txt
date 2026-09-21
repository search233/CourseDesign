cmake_minimum_required(VERSION 3.16)
project(Ethernet8023_TestRunner LANGUAGES CXX)

# ==========================================
# 1. 编译标准设置: C++20
# ==========================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==========================================
# 2. 链接器设置: 使用 mold
# ==========================================
if(NOT MSVC)
    add_link_options("-fuse-ld=mold")
    add_compile_options(-Wall -Wextra)
endif()

# ==========================================
# 3. 头文件搜索路径设置
# ==========================================
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${SOURCE_DIR})

# ==========================================
# 4. Core 核心静态库构建
# ==========================================
add_library(EthernetCore STATIC
    ${SOURCE_DIR}/core/frame_type.cc
    ${SOURCE_DIR}/core/utils.cc
    ${SOURCE_DIR}/core/crc_engine.cc
    ${SOURCE_DIR}/core/frame_builder.cc
)

# ==========================================
# 5. 测试程序构建
# ==========================================
add_executable(test_runner test/test_runner.cc)
target_link_libraries(test_runner PRIVATE EthernetCore)

# 将 test_cases.ini 从源码目录自动复制到构建输出目录，使得程序运行时能直接找到
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/test_cases.ini 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})