#!/bin/bash

# SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
# SPDX-License-Identifier: MIT

# 示例：如何使用 Qt 版本检测和适配工具
# Example: How to use Qt version detection and adaptation tools

echo "=== Qt 版本检测和适配示例 ==="
echo "=== Qt Version Detection and Adaptation Example ==="
echo ""

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

echo "1. 检测系统中可用的 Qt 版本"
echo "1. Detect available Qt versions in the system"
echo "   命令: ./tools/detect_qt_versions.sh"
echo "   Command: ./tools/detect_qt_versions.sh"
echo ""

./tools/detect_qt_versions.sh

echo ""
echo "2. 查看生成的构建配置"
echo "2. View generated build configuration"
echo "   文件: build_config.mk"
echo "   File: build_config.mk"
echo ""

if [ -f "build_config.mk" ]; then
    echo "构建配置内容:"
    echo "Build configuration content:"
    cat build_config.mk
else
    echo "构建配置文件不存在"
    echo "Build configuration file does not exist"
fi

echo ""
echo "3. 使用 debian/rules 检测 Qt 版本"
echo "3. Use debian/rules to detect Qt versions"
echo "   命令: debian/rules print-qt-versions"
echo "   Command: debian/rules print-qt-versions"
echo ""

debian/rules print-qt-versions

echo ""
echo "4. 手动构建特定版本"
echo "4. Manually build specific versions"
echo ""

# 检查 build_config.mk 中的配置
if grep -q "BUILD_QT5 := yes" build_config.mk 2>/dev/null; then
    echo "构建 Qt5 版本:"
    echo "Building Qt5 version:"
    echo "  mkdir -p build-qt5"
    echo "  cd build-qt5"
    echo "  QT_SELECT=5 qmake PREFIX=/usr ../media-analyzer.pro"
    echo "  make"
    echo ""
fi

if grep -q "BUILD_QT6 := yes" build_config.mk 2>/dev/null; then
    echo "构建 Qt6 版本:"
    echo "Building Qt6 version:"
    echo "  mkdir -p build-qt6"
    echo "  cd build-qt6"
    echo "  qmake6 PREFIX=/usr ../media-analyzer.pro"
    echo "  make"
    echo ""
fi

echo "5. 使用自动构建脚本"
echo "5. Use auto-build script"
echo "   命令: ./tools/build_with_qt_detection.sh"
echo "   Command: ./tools/build_with_qt_detection.sh"
echo ""

echo "注意: 这个示例不会实际构建项目，只是展示如何使用这些工具"
echo "Note: This example doesn't actually build the project, just shows how to use these tools"

echo ""
echo "=== 示例结束 ==="
echo "=== Example End ==="