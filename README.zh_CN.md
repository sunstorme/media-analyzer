# MediaAnalyzer

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/version-1.1.0-blue.svg)](https://gitee.com/sunstom/media-analyzer)
[![Qt Version](https://img.shields.io/badge/Qt-5.15%2B%20%7C%206.x-green.svg)](https://www.qt.io)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](https://gitee.com/sunstom/media-analyzer)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://gitee.com/sunstom/media-analyzer)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)

MediaAnalyzer 是一个基于 Qt 的多媒体文件分析调试工具，使用 FFprobe 通过图形界面提供媒体文件的详细信息。

## 功能特性

- **全面的媒体分析**：通过 FFprobe 集成获取详细的格式、编码器和流信息
- **高级UI组件**：媒体播放器、搜索面板、日志查看器、JSON格式化工具和表格视图
- **核心功能**：状态同步、可配置日志、文件历史、导出功能、多语言支持
- **Qt5/Qt6兼容性**：完全支持 Qt5 和 Qt6，具有自动检测功能

## 安装

### 系统要求

- **Qt**: Qt 5.15+ 或 Qt 6.x
- **FFmpeg**: 包含开发库和 pkg-config 文件
- **编译器**: 支持 C++17
- **CMake**: 3.16 或更高版本

### Linux 依赖

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake pkg-config
sudo apt install qt5-default qttools5-dev qt6-base-dev qt6-tools-dev
sudo apt install libavcodec-dev libavformat-dev libavutil-dev
sudo apt install libswscale-dev libswresample-dev libavfilter-dev
sudo apt install libx11-dev
```

## 构建方法

### 方法1：使用构建脚本（推荐）

```bash
# 基本构建
./build.sh

# 调试构建
./build.sh -d

# 指定Qt版本
./build.sh -q 6

# 清理构建
./build.sh -c

# 查看所有选项
./build.sh -h
```

### 方法2：Qt版本检测和适配

对于同时安装了 Qt5 和 Qt6 的系统：

```bash
# 检测可用的Qt版本
./tools/detect_qt_versions.sh

# 带检测的自动构建
./tools/build_with_qt_detection.sh
```

### 方法3：手动CMake构建

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Qt版本检测

项目为多Qt环境提供了增强工具：

- **自动检测**：检测 Qt5 和 Qt6 的可用性
- **条件构建**：只构建可用的 Qt 版本
- **Debian打包**：为多Qt支持优化的 `debian/rules`

详细信息请参阅 [`docs/qt_version_detection.md`](docs/qt_version_detection.md:1)。

## 使用方法

1. **文件操作**：通过菜单或拖放打开媒体文件
2. **媒体分析**：查看详细的格式信息、流和编解码器
3. **高级功能**：使用嵌入式播放器、搜索和日志分析

## 项目结构

```
media-analyzer/
├── src/                    # 源代码
│   ├── common/            # 核心组件
│   ├── widgets/           # UI组件
│   └── model/             # 数据模型
├── tools/                 # 构建和工具脚本
├── docs/                  # 文档
├── assets/                # 资源和图标
└── debian/                # Debian打包
```

## 故障排除

### 常见问题

**找不到Qt**：
```bash
# 安装Qt开发包
sudo apt install qt6-base-dev qt6-tools-dev  # Qt6
sudo apt install qt5-default qttools5-dev    # Qt5
```

**找不到FFmpeg**：
```bash
sudo apt install libavcodec-dev libavformat-dev libavutil-dev
sudo apt install libswscale-dev libswresample-dev libavfilter-dev
```

## 开发

### Qt5/Qt6兼容性

项目使用 [`src/common/qtcompat.h`](src/common/qtcompat.h:1) 中的兼容性层来处理 Qt5 和 Qt6 之间的 API 差异。

### 贡献指南

1. Fork 仓库
2. 创建功能分支
3. 遵循编码标准
4. 测试 Qt5/Qt6 兼容性
5. 提交 pull request

## 许可证

本项目采用 MIT 许可证。详情请参阅 [LICENSE](LICENSE) 文件。
