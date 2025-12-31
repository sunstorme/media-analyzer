# Qt 版本检测和适配

本文档说明如何在系统中同时存在 Qt5 和 Qt6 的情况下构建 media-analyzer 项目。

## 概述

media-analyzer 项目支持 Qt5 和 Qt6 两个版本。在系统中可能同时存在这两个版本的 Qt，因此需要检测可用的 Qt 版本并相应地构建项目。

## 检测 Qt 版本

### 使用检测脚本

项目提供了 `tools/detect_qt_versions.sh` 脚本来检测系统中可用的 Qt 版本：

```bash
./tools/detect_qt_versions.sh
```

该脚本会：
- 检测系统中是否安装了 Qt5 和 Qt6
- 显示每个 Qt 版本的详细信息
- 生成 `build_config.mk` 文件，包含构建配置信息
- 提供构建建议

### 手动检测

您也可以手动检测 Qt 版本：

```bash
# 检测 Qt5
export QT_SELECT=5
qmake --version

# 检测 Qt6
qmake6 --version
```

## 构建项目

### 使用自动构建脚本

项目提供了 `tools/build_with_qt_detection.sh` 脚本，它会自动检测 Qt 版本并构建相应的版本：

```bash
./tools/build_with_qt_detection.sh
```

该脚本会：
- 检测系统中可用的 Qt 版本
- 清理之前的构建目录
- 构建所有可用的 Qt 版本
- 显示构建结果和运行说明

### 手动构建

您也可以手动构建特定版本：

#### 构建 Qt5 版本

```bash
mkdir -p build-qt5
cd build-qt5
export QT_SELECT=5
qmake PREFIX=/usr ../media-analyzer.pro
make
cd ..
```

#### 构建 Qt6 版本

```bash
mkdir -p build-qt6
cd build-qt6
qmake6 PREFIX=/usr ../media-analyzer.pro
make
cd ..
```

## Debian 包构建

### 优化的 debian/rules

项目的 `debian/rules` 文件已经优化，可以自动检测系统中可用的 Qt 版本：

```makefile
# 检测可用的 Qt 版本
QT5_AVAILABLE := $(shell QT_SELECT=5 qmake --version 2>/dev/null && echo yes)
QT6_AVAILABLE := $(shell qmake6 --version 2>/dev/null && echo yes)
```

构建 Debian 包时，只有系统中可用的 Qt 版本会被构建：

```bash
dpkg-buildpackage -us -uc
```

## 安装 Qt 开发包

如果系统中没有安装 Qt 开发包，可以使用以下命令安装：

### 安装 Qt5 开发包

```bash
sudo apt-get install qtbase5-dev qttools5-dev-tools
```

### 安装 Qt6 开发包

```bash
sudo apt-get install qt6-base-dev qt6-tools-dev-tools
```

## 项目配置

### .pro 文件配置

项目的 `media-analyzer.pro` 文件已经配置为根据 Qt 版本自动设置不同的目标名称：

```qmake
# 根据不同的 Qt 版本设置不同的目标名称
equals(QT_MAJOR_VERSION, 5) {
    TARGET = media-analyzer
} else {
    TARGET = media-analyzer6
}
```

### 翻译支持

目前只有 Qt5 版本支持翻译：

```qmake
# Qt6 没有 lrelease 工具，暂时禁用翻译
equals(QT_MAJOR_VERSION, 5) {
    CONFIG += lrelease
    CONFIG += embed_translations
}
```

## 故障排除

### 常见问题

1. **找不到 qmake 或 qmake6**
   - 确保已安装相应的 Qt 开发包
   - 检查 PATH 环境变量

2. **构建失败**
   - 检查是否安装了所有必需的依赖项
   - 查看 `media-analyzer.pro` 文件中的依赖项配置

3. **版本冲突**
   - 使用 `QT_SELECT` 环境变量指定 Qt 版本
   - 使用特定版本的 qmake 命令（qmake 或 qmake6）

### 调试信息

使用 debian/rules 中的调试目标查看检测到的 Qt 版本：

```bash
debian/rules print-qt-versions
```

## 总结

通过使用提供的脚本和优化的构建配置，media-analyzer 项目可以灵活地适配系统中同时存在 Qt5 和 Qt6 的情况。构建系统会自动检测可用的 Qt 版本并相应地构建项目，无需手动干预。