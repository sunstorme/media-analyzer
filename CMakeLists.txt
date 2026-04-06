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
    translations/media-analyzer_zh_CN.ts
    translations/media-analyzer_en_US.ts
)

# Add QJsonModel subdirectory
add_subdirectory(third_part/QJsonModel)

# Create executable
# Use the same executable name for both Qt5 and Qt6
set(EXECUTABLE_NAME media-analyzer)

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

# Process translation files
# Note: Translation files are handled manually using lupdate/lrelease tools
# To update translations: QT_SELECT=qt5 lupdate media-analyzer.pro -no-obsolete
# To generate .qm files: QT_SELECT=qt5 lrelease translations/*.ts

if(QT_MAJOR_VERSION EQUAL 5)
    # Qt5: Use QT5_ADD_TRANSLATION to create .qm files from .ts files
    QT5_ADD_TRANSLATION(QM_FILES ${TRANSLATION_FILES})
    
    # Add .qm files to the executable
    target_sources(${EXECUTABLE_NAME} PRIVATE ${QM_FILES})
else()
    # Qt6: Create custom target to generate .qm files
    set(QM_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/media-analyzer_zh_CN.qm
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/media-analyzer_en_US.qm
    )
    
    # Use the actual Qt6 lrelease path directly
    set(LRELEASE_EXECUTABLE "/usr/lib/qt6/bin/lrelease")
    
    if(EXISTS "${LRELEASE_EXECUTABLE}")
        message(STATUS "Found lrelease: ${LRELEASE_EXECUTABLE}")
        add_custom_target(lrelease ALL
            COMMAND ${LRELEASE_EXECUTABLE} ${TRANSLATION_FILES}
            COMMENT "Releasing translation files"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            BYPRODUCTS ${QM_FILES}
        )
        
        # Make the executable depend on the lrelease target
        add_dependencies(${EXECUTABLE_NAME} lrelease)
    else()
        message(WARNING "lrelease not found at ${LRELEASE_EXECUTABLE}. Skipping automatic translation generation.")
        message(WARNING "Please generate .qm files manually: /usr/lib/qt6/bin/lrelease translations/*.ts")
    endif()
endif()

# Copy translation files to build directory for development
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/translations
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        ${CMAKE_SOURCE_DIR}/translations/media-analyzer_zh_CN.qm
        ${CMAKE_SOURCE_DIR}/translations/media-analyzer_en_US.qm
        ${CMAKE_BINARY_DIR}/bin/translations/
    COMMENT "Copying translation files to build directory"
)

# Installation configuration
# Set these BEFORE including GNUInstallDirs to override defaults
if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()

# Force consistent installation directories for both Qt5 and Qt6
set(CMAKE_INSTALL_BINDIR bin CACHE PATH "User executables")
set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "Object code libraries")
set(CMAKE_INSTALL_INCLUDEDIR include CACHE PATH "C header files")

include(GNUInstallDirs)

set(BINDIR ${CMAKE_INSTALL_BINDIR})
set(ICONDIR ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
set(APPDIR ${CMAKE_INSTALL_DATADIR}/applications)
set(DOCDIR ${CMAKE_INSTALL_DOCDIR})

# Install executable
install(TARGETS ${EXECUTABLE_NAME}
    RUNTIME DESTINATION bin
)

# Install icon
install(FILES assets/128x128/media-analyzer-logo.svg
    DESTINATION ${ICONDIR}
    RENAME media-analyzer.svg
)

# Install desktop file (different file for Qt5 and Qt6)
if(QT_MAJOR_VERSION EQUAL 6)
    install(FILES assets/media-analyzer6.desktop
        DESTINATION ${APPDIR}
        RENAME media-analyzer.desktop
    )
else()
    install(FILES assets/media-analyzer.desktop
        DESTINATION ${APPDIR}
    )
endif()

# Install documentation
install(FILES README.md LICENSE
    DESTINATION ${DOCDIR}
)

# Install translation files
# Manually install .qm files for both Qt5 and Qt6
install(FILES 
    translations/media-analyzer_zh_CN.qm
    translations/media-analyzer_en_US.qm
    DESTINATION ${CMAKE_INSTALL_DATADIR}/media-analyzer/translations
    COMPONENT Runtime
)

# Print configuration information
message(STATUS "")
message(STATUS "MediaAnalyzer Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Qt Version: ${QT_MAJOR_VERSION}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Executable Name: ${EXECUTABLE_NAME}")
message(STATUS "")