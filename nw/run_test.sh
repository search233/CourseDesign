#!/bin/bash

# 开启报错即退出机制，确保任何一步编译失败时脚本都能及时中止
set -e

echo "=========================================="
echo "   IEEE 802.3 帧封装与测试自动化脚本      "
echo "=========================================="

# 1. 创建并进入 build 目录
BUILD_DIR="build"
if [ ! -d "$BUILD_DIR" ]; then
    echo "[信息] 创建构建目录: $BUILD_DIR"
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# 2. 运行 CMake 配置
echo "[1/3] 正在运行 CMake 配置..."
cmake ..

# 3. 自动同步 compile_commands.json 到根目录（供 clangd 读取）
if [ -f "compile_commands.json" ]; then
    ln -sf build/compile_commands.json ../compile_commands.json
    echo "[提示] 已自动更新根目录下的 compile_commands.json，clangd 语法高亮已同步。"
fi

# 4. 使用 CMake 跨平台统一构建命令（自动使用多核并行编译）
echo "[2/3] 正在编译项目..."
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

# 5. 执行测试二进制程序
echo "[3/3] 正在执行测试套件..."
echo "------------------------------------------"
./test_runner

echo "=========================================="
echo "   构建与测试流程全部顺利完成！         "
echo "=========================================="