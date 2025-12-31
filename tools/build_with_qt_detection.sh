#!/bin/bash

# SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
# SPDX-License-Identifier: MIT

# 自动检测 Qt 版本并构建项目
# Automatically detect Qt versions and build the project

set -e

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

echo "项目根目录: $PROJECT_ROOT"
echo "Project root: $PROJECT_ROOT"

# 检测 Qt 版本
echo ""
echo "=== 检测 Qt 版本 ==="
echo "=== Detecting Qt Versions ==="

QT5_AVAILABLE=false
QT6_AVAILABLE=false

# 检测 Qt5
if QT_SELECT=5 qmake --version >/dev/null 2>&1; then
    echo "Qt5 可用: $(QT_SELECT=5 qmake --version 2>&1 | head -1)"
    QT5_AVAILABLE=true
else
    echo "Qt5 不可用"
fi

# 检测 Qt6
if command -v qmake6 >/dev/null 2>&1; then
    echo "Qt6 可用: $(qmake6 --version 2>&1 | head -1)"
    QT6_AVAILABLE=true
else
    echo "Qt6 不可用"
fi

# 检查是否有可用的 Qt 版本
if [ "$QT5_AVAILABLE" = false ] && [ "$QT6_AVAILABLE" = false ]; then
    echo ""
    echo "错误: 系统中没有检测到可用的 Qt 版本 (Qt5 或 Qt6)"
    echo "Error: No available Qt versions detected in the system (Qt5 or Qt6)"
    echo "请安装 Qt5 或 Qt6 开发包:"
    echo "Please install Qt5 or Qt6 development packages:"
    echo "  对于 Qt5/For Qt5: apt-get install qtbase5-dev qttools5-dev-tools"
    echo "  对于 Qt6/For Qt6: apt-get install qt6-base-dev qt6-tools-dev-tools"
    exit 1
fi

# 清理之前的构建
echo ""
echo "=== 清理之前的构建 ==="
echo "=== Cleaning previous builds ==="
rm -rf build-qt5 build-qt6

# 构建 Qt5 版本
if [ "$QT5_AVAILABLE" = true ]; then
    echo ""
    echo "=== 构建 Qt5 版本 ==="
    echo "=== Building Qt5 version ==="
    mkdir -p build-qt5
    cd build-qt5
    QT_SELECT=5 qmake PREFIX=/usr ../media-analyzer.pro
    make -j$(nproc)
    cd "$PROJECT_ROOT"
    echo "Qt5 版本构建完成"
    echo "Qt5 version build completed"
fi

# 构建 Qt6 版本
if [ "$QT6_AVAILABLE" = true ]; then
    echo ""
    echo "=== 构建 Qt6 版本 ==="
    echo "=== Building Qt6 version ==="
    mkdir -p build-qt6
    cd build-qt6
    qmake6 PREFIX=/usr ../media-analyzer.pro
    make -j$(nproc)
    cd "$PROJECT_ROOT"
    echo "Qt6 版本构建完成"
    echo "Qt6 version build completed"
fi

echo ""
echo "=== 构建完成 ==="
echo "=== Build completed ==="

# 显示构建结果
if [ "$QT5_AVAILABLE" = true ]; then
    echo "Qt5 版本可执行文件: build-qt5/media-analyzer"
    echo "Qt5 executable: build-qt5/media-analyzer"
fi

if [ "$QT6_AVAILABLE" = true ]; then
    echo "Qt6 版本可执行文件: build-qt6/media-analyzer6"
    echo "Qt6 executable: build-qt6/media-analyzer6"
fi

echo ""
echo "运行程序:"
echo "To run the application:"
if [ "$QT5_AVAILABLE" = true ]; then
    echo "  Qt5: ./build-qt5/media-analyzer"
fi
if [ "$QT6_AVAILABLE" = true ]; then
    echo "  Qt6: ./build-qt6/media-analyzer6"
fi