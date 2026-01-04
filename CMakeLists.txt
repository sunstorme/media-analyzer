# SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)

# Project information
project(MediaAnalyzer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include CMake module paths
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Qt version detection and configuration
include(FindQtCompat)

# Enable Qt MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set include paths
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/third_part/QJsonModel/include)

# Collect source files
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

# Merge UI files
list(APPEND UI_FILES ${FORM_FILES})

# Resource files
set(RESOURCES_FILES
    assets/resources.qrc
)

# Translation files
set(TRANSLATION_FILES
    translations/media-analyzer_en_US.ts
    translations/media-analyzer_zh_CN.ts
)

# Add QJsonModel subdirectory
add_subdirectory(third_part/QJsonModel)

# Create executable
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

# Link Qt libraries
target_link_libraries(${EXECUTABLE_NAME}
    ${QT_CORE_TARGET}
    ${QT_WIDGETS_TARGET}
    ${QT_GUI_TARGET}
    ${QT_CONCURRENT_TARGET}
)

# Link QJsonModel
target_link_libraries(${EXECUTABLE_NAME} QJsonModel)

# Set compile definitions
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

# Installation configuration
include(GNUInstallDirs)

if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

set(BINDIR ${CMAKE_INSTALL_BINDIR})
set(ICONDIR ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
set(APPDIR ${CMAKE_INSTALL_DATADIR}/applications)
set(DOCDIR ${CMAKE_INSTALL_DOCDIR})

# Install executable
install(TARGETS ${EXECUTABLE_NAME}
    RUNTIME DESTINATION ${BINDIR}
)

# Install icon
install(FILES assets/128x128/media-analyzer-logo.svg
    DESTINATION ${ICONDIR}
    RENAME media-analyzer.svg
)

# Install desktop file
install(FILES assets/media-analyzer.desktop
    DESTINATION ${APPDIR}
)

# Install documentation
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

# Print configuration information
message(STATUS "")
message(STATUS "MediaAnalyzer Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Qt Version: ${QT_MAJOR_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Executable Name: ${EXECUTABLE_NAME}")
message(STATUS "")