#!/bin/bash

# SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
# SPDX-License-Identifier: MIT

# 检测系统中可用的 Qt 版本
# Detect available Qt versions in the system

echo "检测系统中可用的 Qt 版本 (Qt5 和 Qt6)..."
echo "Detecting available Qt versions in the system (Qt5 and Qt6)..."

# 检测 Qt5
echo ""
echo "检查 Qt5..."
if QT_SELECT=5 qmake --version >/dev/null 2>&1; then
    echo "Qt5 可用: $(QT_SELECT=5 qmake --version 2>&1 | head -1)"
    QT5_AVAILABLE=true
else
    echo "Qt5 不可用"
    QT5_AVAILABLE=false
fi

# 检测 Qt6
echo ""
echo "检查 Qt6..."
if command -v qmake6 >/dev/null 2>&1; then
    echo "Qt6 可用: $(qmake6 --version 2>&1 | head -1)"
    QT6_AVAILABLE=true
else
    echo "Qt6 不可用"
    QT6_AVAILABLE=false
fi

echo ""
echo "=== 检测结果 ==="
echo "=== Detection Results ==="

# 生成构建配置
if [ "$QT5_AVAILABLE" = true ]; then
    echo "BUILD_QT5 := yes" > build_config.mk
else
    echo "BUILD_QT5 := no" > build_config.mk
fi

if [ "$QT6_AVAILABLE" = true ]; then
    echo "BUILD_QT6 := yes" >> build_config.mk
else
    echo "BUILD_QT6 := no" >> build_config.mk
fi

echo "构建配置已写入 build_config.mk"
echo "Build configuration written to build_config.mk"

# 提供构建建议
echo ""
echo "=== 构建建议 ==="
echo "=== Build Recommendations ==="

if [ "$QT5_AVAILABLE" = true ]; then
    echo "可以使用以下命令构建 Qt5 版本:"
    echo "You can build the Qt5 version with:"
    echo "  mkdir -p build-qt5 && cd build-qt5 && QT_SELECT=5 qmake PREFIX=/usr ../media-analyzer.pro && make"
fi

if [ "$QT6_AVAILABLE" = true ]; then
    echo "可以使用以下命令构建 Qt6 版本:"
    echo "You can build the Qt6 version with:"
    echo "  mkdir -p build-qt6 && cd build-qt6 && qmake6 PREFIX=/usr ../media-analyzer.pro && make"
fi

if [ "$QT5_AVAILABLE" = false ] && [ "$QT6_AVAILABLE" = false ]; then
    echo "错误: 系统中没有检测到可用的 Qt 版本 (Qt5 或 Qt6)"
    echo "Error: No available Qt versions detected in the system (Qt5 or Qt6)"
    echo "请安装 Qt5 或 Qt6 开发包:"
    echo "Please install Qt5 or Qt6 development packages:"
    echo "  对于 Qt5/For Qt5: apt-get install qtbase5-dev qttools5-dev-tools"
    echo "  对于 Qt6/For Qt6: apt-get install qt6-base-dev qt6-tools-dev-tools"
    exit 1
fi