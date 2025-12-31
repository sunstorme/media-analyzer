# SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)

# 项目信息
project(MediaAnalyzer VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含CMake模块路径
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Qt版本检测和配置
include(FindQtCompat)

# 启用Qt MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 设置包含路径
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/third_part/QJsonModel/include)

# 查找FFmpeg库
include(FindFFmpegCompat)

# X11支持（Linux）
if(UNIX AND NOT APPLE)
    find_package(X11)
    if(X11_FOUND)
        set(X11_LIBRARIES ${X11_LIBRARIES})
    endif()
endif()

# 收集源文件
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.h"
)

file(GLOB UI_FILES
    "src/*.ui"
)

file(GLOB_RECURSE FORM_FILES
    "src/widgets/*.ui"
)

# 合并UI文件
list(APPEND UI_FILES ${FORM_FILES})

# 资源文件
set(RESOURCES_FILES
    assets/resources.qrc
)

# 翻译文件
set(TRANSLATION_FILES
    translations/media-analyzer_en_US.ts
    translations/media-analyzer_zh_CN.ts
)

# 添加QJsonModel子目录
add_subdirectory(third_part/QJsonModel)

# 创建可执行文件
if(QT_MAJOR_VERSION EQUAL 6)
    set(EXECUTABLE_NAME media-analyzer-qt6)
else()
    set(EXECUTABLE_NAME media-analyzer-qt5)
endif()

add_executable(${EXECUTABLE_NAME}
    ${SOURCES}
    ${UI_FILES}
    ${RESOURCES_FILES}
)

# 链接Qt库
target_link_libraries(${EXECUTABLE_NAME}
    ${QT_CORE_TARGET}
    ${QT_WIDGETS_TARGET}
    ${QT_GUI_TARGET}
    ${QT_CONCURRENT_TARGET}
)

# 链接QJsonModel
target_link_libraries(${EXECUTABLE_NAME} QJsonModel)

# 链接FFmpeg库
target_link_libraries(${EXECUTABLE_NAME} FFMpeg::FFmpeg)

# 链接X11库（Linux）
if(UNIX AND NOT APPLE AND X11_FOUND)
    target_link_libraries(${EXECUTABLE_NAME} ${X11_LIBRARIES})
endif()

# 设置编译定义
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
    ${QT_COMPAT_DEFINES}
)

# Temporarily skip translation file processing to avoid Qt5/Qt6 compatibility issues
# TODO: Re-enable after fixing translation functionality
# if(QT_MAJOR_VERSION EQUAL 6)
#     qt_compat_add_lupdate(${EXECUTABLE_NAME} ${SOURCES} ${UI_FILES} ${TRANSLATION_FILES})
#     qt_compat_add_lrelease(${EXECUTABLE_NAME} ${TRANSLATION_FILES})
# else()
#     # Qt5 uses different function signatures
#     qt_compat_add_lupdate(QM_FILES ${SOURCES} ${UI_FILES} ${TRANSLATION_FILES})
#     qt_compat_add_lrelease(QM_FILES ${TRANSLATION_FILES})
# endif()

# 安装配置
include(GNUInstallDirs)

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

set(BINDIR ${CMAKE_INSTALL_BINDIR})
set(ICONDIR ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
set(APPDIR ${CMAKE_INSTALL_DATADIR}/applications)
set(DOCDIR ${CMAKE_INSTALL_DOCDIR})

# 安装可执行文件
install(TARGETS ${EXECUTABLE_NAME}
    RUNTIME DESTINATION ${BINDIR}
)

# 安装图标
install(FILES assets/128x128/media-analyzer-logo.svg
    DESTINATION ${ICONDIR}
    RENAME media-analyzer.svg
)

# 安装桌面文件
install(FILES assets/media-analyzer.desktop
    DESTINATION ${APPDIR}
)

# 安装文档
install(FILES README.md LICENSE
    DESTINATION ${DOCDIR}
)

# Temporarily skip translation file installation to avoid Qt5/Qt6 compatibility issues
# TODO: Re-enable after fixing translation functionality
# if(QT_MAJOR_VERSION EQUAL 6)
#     qt_install_translations(
#         ${EXECUTABLE_NAME}
#         ${TRANSLATION_FILES}
#         DESTINATION ${CMAKE_INSTALL_DATADIR}/media-analyzer/translations
#     )
# else()
#     qt5_install_translations(
#         ${EXECUTABLE_NAME}
#         ${TRANSLATION_FILES}
#         DESTINATION ${CMAKE_INSTALL_DATADIR}/media-analyzer/translations
#     )
# endif()

# 打印配置信息
message(STATUS "")
message(STATUS "MediaAnalyzer Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Qt Version: ${QT_MAJOR_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Executable Name: ${EXECUTABLE_NAME}")
message(STATUS "")