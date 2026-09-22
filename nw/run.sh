#!/bin/bash
set -e

# 项目根目录
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

# 获取 CPU 核心数
NUM_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

# 确保 build 目录与 compile_commands 软链接
configure_if_needed() {
    if [ ! -f "${BUILD_DIR}/Makefile" ] && [ ! -f "${BUILD_DIR}/build.ninja" ]; then
        echo "[1/2] 正在初始化 CMake 配置..."
        cmake -B "${BUILD_DIR}" -S "${PROJECT_DIR}"
    fi
    # 软链接 compile_commands.json 供 clangd 使用
    if [ -f "${BUILD_DIR}/compile_commands.json" ]; then
        ln -sf build/compile_commands.json "${PROJECT_DIR}/compile_commands.json"
    fi
}

# 动作分发
ACTION="${1:-tui}"

case "${ACTION}" in
    tui|run)
        configure_if_needed
        cmake --build "${BUILD_DIR}" --target ethernet_tui -j"${NUM_CORES}"
        echo "[2/2] 正在全屏启动 IEEE 802.3 仿真器..."
        exec "${BUILD_DIR}/ethernet_tui"
        ;;
    test)
        configure_if_needed
        cmake --build "${BUILD_DIR}" --target test_runner -j"${NUM_CORES}"
        (cd "${BUILD_DIR}" && ./test_runner)
        ;;
    build)
        configure_if_needed
        cmake --build "${BUILD_DIR}" -j"${NUM_CORES}"
        echo "编译完成！"
        ;;
    clean)
        echo "正在清理构建目录..."
        rm -rf "${BUILD_DIR}"
        echo "清理完毕。"
        ;;
    help|-h|--help)
        echo "================================================="
        echo "  IEEE 802.3 仿真器与测试便捷启动器              "
        echo "================================================="
        echo "用法: ./run.sh [命令]"
        echo ""
        echo "常用命令:"
        echo "  ./run.sh          - (默认) 自动增量构建并直接启动仿真器 TUI"
        echo "  ./run.sh test     - 构建并运行 12 组自动化测试套件"
        echo "  ./run.sh build    - 仅编译所有目标"
        echo "  ./run.sh clean    - 清理构建目录 (build/)"
        echo "  ./run.sh help     - 查看帮助说明"
        ;;
    *)
        echo "未知命令: ${ACTION}"
        echo "请运行 ./run.sh help 查看支持的命令"
        exit 1
        ;;
esac
