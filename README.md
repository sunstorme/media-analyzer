# MediaAnalyzer

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/version-1.1.0-blue.svg)](https://gitee.com/sunstom/media-analyzer)
[![Qt Version](https://img.shields.io/badge/Qt-5.15%2B%20%7C%206.x-green.svg)](https://www.qt.io)
[![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)](https://gitee.com/sunstom/media-analyzer)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://gitee.com/sunstom/media-analyzer)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)

MediaAnalyzer is a Qt-based multimedia file analysis and debugging tool that uses FFprobe to provide detailed information about media files through a graphical interface.

## Features

- **Comprehensive Media Analysis**: Detailed format, encoder and stream information via FFprobe integration
- **Advanced UI Components**: Media player, search panel, log viewer, JSON formatter, and table view
- **Core Functionality**: State synchronization, configurable logging, file history, export capabilities, multi-language support
- **Qt5/Qt6 Compatibility**: Full support for both Qt5 and Qt6 with automatic detection

## Installation

### System Requirements

- **Qt**: Qt 5.15+ or Qt 6.x
- **FFmpeg**: Including development libraries and pkg-config files
- **Compiler**: C++17 compatible
- **CMake**: 3.16 or higher

### Linux Dependencies

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake pkg-config
sudo apt install qt5-default qttools5-dev qt6-base-dev qt6-tools-dev
```

## Build Methods

### Method 1: Using Build Script (Recommended)

```bash
# Basic build
./build.sh

# Debug build
./build.sh -d

# Specify Qt version
./build.sh -q 6

# Clean build
./build.sh -c

# View all options
./build.sh -h
```

### Method 2: Qt Version Detection and Adaptation

For systems with both Qt5 and Qt6:

```bash
# Detect available Qt versions
./tools/detect_qt_versions.sh

# Auto-build with detection
./tools/build_with_qt_detection.sh
```

### Method 3: Manual CMake Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Qt Version Detection

The project provides enhanced tools for multi-Qt environments:

- **Automatic Detection**: Detects Qt5 and Qt6 availability
- **Conditional Building**: Only builds available Qt versions
- **Debian Packaging**: Optimized `debian/rules` for multi-Qt support

For detailed information, see [`docs/qt_version_detection.md`](docs/qt_version_detection.md:1).

## Usage

1. **File Operations**: Open media files via menu or drag-and-drop
2. **Media Analysis**: View detailed format info, streams, and codecs
3. **Advanced Features**: Use embedded player, search, and log analysis

## Project Structure

```
media-analyzer/
├── src/                    # Source code
│   ├── common/            # Core components
│   ├── widgets/           # UI components
│   └── model/             # Data models
├── tools/                 # Build and utility scripts
├── docs/                  # Documentation
├── assets/                # Resources and icons
└── debian/                # Debian packaging
```

## Troubleshooting

### Common Issues

**Qt Not Found**:
```bash
# Install Qt development packages
sudo apt install qt6-base-dev qt6-tools-dev  # Qt6
sudo apt install qt5-default qttools5-dev    # Qt5
```

## Development

### Qt5/Qt6 Compatibility

The project uses a compatibility layer in [`src/common/qtcompat.h`](src/common/qtcompat.h:1) to handle API differences between Qt5 and Qt6.

### Contributing

1. Fork the repository
2. Create a feature branch
3. Follow coding standards
4. Test Qt5/Qt6 compatibility
5. Submit a pull request

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
