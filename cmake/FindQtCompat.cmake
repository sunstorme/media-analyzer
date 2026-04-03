# SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
# SPDX-License-Identifier: MIT

# FindQtCompat.cmake - Qt5/Qt6 compatibility detection module

# Check if QT_MAJOR_VERSION is already defined
if(NOT DEFINED QT_MAJOR_VERSION)
    # Allow user to specify Qt version
    set(QT_VERSION_MAJOR "" CACHE STRING "Qt major version to use (5 or 6)")
    
    if(QT_VERSION_MAJOR)
        set(QT_MAJOR_VERSION ${QT_VERSION_MAJOR})
        message(STATUS "Using user-specified Qt version: ${QT_MAJOR_VERSION}")
    else()
        # Auto-detect Qt version, prefer Qt6
        find_package(Qt6 QUIET COMPONENTS Core Widgets Gui Concurrent)
        if(Qt6_FOUND)
            set(QT_MAJOR_VERSION 6)
            message(STATUS "Auto-detected Qt version: 6")
        else()
            find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui Concurrent)
            set(QT_MAJOR_VERSION 5)
            message(STATUS "Auto-detected Qt version: 5")
        endif()
    endif()
endif()

# Set variables based on Qt version
if(QT_MAJOR_VERSION EQUAL 6)
    # Qt6 configuration
    if(NOT Qt6_FOUND)
        find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Concurrent)
        find_package(Qt6LinguistTools REQUIRED)
    endif()
    
    set(QT_TARGET_PREFIX Qt6)
    set(QT_CORE_TARGET Qt6::Core)
    set(QT_WIDGETS_TARGET Qt6::Widgets)
    set(QT_GUI_TARGET Qt6::Gui)
    set(QT_CONCURRENT_TARGET Qt6::Concurrent)
    set(QT_LINGUISTTOOLS_TARGET Qt6::LinguistTools)
    
    # Qt6 macro functions
    function(qt_compat_add_lupdate)
        qt_add_lupdate(${ARGN})
    endfunction()
    
    function(qt_compat_add_lrelease)
        qt_add_lrelease(${ARGN})
    endfunction()
    
    function(qt_compat_install_translations)
        qt_install_translations(${ARGN})
    endfunction()
    
    function(qt_compat_standard_project_setup)
        qt_standard_project_setup()
    endfunction()
    
elseif(QT_MAJOR_VERSION EQUAL 5)
    # Qt5 configuration
    if(NOT Qt5_FOUND)
        find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui Concurrent LinguistTools)
    endif()
    
    set(QT_TARGET_PREFIX Qt5)
    set(QT_CORE_TARGET Qt5::Core)
    set(QT_WIDGETS_TARGET Qt5::Widgets)
    set(QT_GUI_TARGET Qt5::Gui)
    set(QT_CONCURRENT_TARGET Qt5::Concurrent)
    set(QT_LINGUISTTOOLS_TARGET Qt5::LinguistTools)
    
    # Qt5 macro functions
    function(qt_compat_add_lupdate)
        QT5_CREATE_TRANSLATION(${ARGN})
    endfunction()
    
    function(qt_compat_add_lrelease)
        QT5_ADD_TRANSLATION(${ARGN})
    endfunction()
    
    function(qt_compat_install_translations)
        qt5_install_translations(${ARGN})
    endfunction()
    
    function(qt_compat_standard_project_setup)
        # Qt5 doesn't need this function
    endfunction()
    
else()
    message(FATAL_ERROR "Unsupported Qt version: ${QT_MAJOR_VERSION}. Only Qt5 and Qt6 are supported.")
endif()

# Set Qt compatibility compile definitions
if(QT_MAJOR_VERSION EQUAL 6)
    set(QT_COMPAT_DEFINES
        "QT_DISABLE_DEPRECATED_BEFORE=0x050F00"  # Qt 5.15
    )
else()
    set(QT_COMPAT_DEFINES
        "QT_DISABLE_DEPRECATED_BEFORE=0x050F00"  # Qt 5.15
    )
endif()

# Export variables to parent scope
set(QT_MAJOR_VERSION ${QT_MAJOR_VERSION} PARENT_SCOPE)
set(QT_TARGET_PREFIX ${QT_TARGET_PREFIX} PARENT_SCOPE)
set(QT_CORE_TARGET ${QT_CORE_TARGET} PARENT_SCOPE)
set(QT_WIDGETS_TARGET ${QT_WIDGETS_TARGET} PARENT_SCOPE)
set(QT_GUI_TARGET ${QT_GUI_TARGET} PARENT_SCOPE)
set(QT_CONCURRENT_TARGET ${QT_CONCURRENT_TARGET} PARENT_SCOPE)
set(QT_LINGUISTTOOLS_TARGET ${QT_LINGUISTTOOLS_TARGET} PARENT_SCOPE)
set(QT_COMPAT_DEFINES ${QT_COMPAT_DEFINES} PARENT_SCOPE)

message(STATUS "Qt compatibility configured:")
message(STATUS "  Qt Version: ${QT_MAJOR_VERSION}")
message(STATUS "  Target Prefix: ${QT_TARGET_PREFIX}")