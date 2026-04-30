# SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
# SPDX-License-Identifier: MIT

# Qt version suffix for output name
if(QT_MAJOR_VERSION EQUAL 6)
    set(QT_VERSION_SUFFIX "-qt6")
else()
    set(QT_VERSION_SUFFIX "-qt5")
endif()

set(BIN_NAME dfm-extension-media${QT_VERSION_SUFFIX})

# Plugin sources
file(GLOB PLUGIN_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

# Shared sources needed by the plugin (auto-collected to avoid missing symbols)
file(GLOB_RECURSE SHARED_COMMON_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/common/*.cpp"
)
file(GLOB SHARED_WIDGET_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/widgets/basefmtwg.cpp"
    "${CMAKE_SOURCE_DIR}/src/widgets/jsonfmtwg.cpp"
    "${CMAKE_SOURCE_DIR}/src/widgets/tablefmtwg.cpp"
    "${CMAKE_SOURCE_DIR}/src/widgets/searchwg.cpp"
    "${CMAKE_SOURCE_DIR}/src/widgets/progressdlg.cpp"
    "${CMAKE_SOURCE_DIR}/src/widgets/helpquerywg.cpp"
    "${CMAKE_SOURCE_DIR}/src/widgets/formatwg.cpp"
    "${CMAKE_SOURCE_DIR}/src/widgets/streamswg.cpp"
    "${CMAKE_SOURCE_DIR}/src/widgets/insertnodedialog.cpp"
)
set(SHARED_MODEL_SOURCES
    ${CMAKE_SOURCE_DIR}/src/model/mediainfotabelmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/model/multicolumnsearchproxymodel.cpp
)
set(SHARED_COMPONENT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/component/zprogressbar.cpp
)
set(SHARED_SOURCES
    ${SHARED_COMMON_SOURCES}
    ${SHARED_WIDGET_SOURCES}
    ${SHARED_MODEL_SOURCES}
    ${SHARED_COMPONENT_SOURCES}
)

# UI files needed by shared widgets (only existing .ui files)
set(SHARED_UI_FILES
    ${CMAKE_SOURCE_DIR}/src/widgets/jsonfmtwg.ui
    ${CMAKE_SOURCE_DIR}/src/widgets/tablefmtwg.ui
    ${CMAKE_SOURCE_DIR}/src/widgets/searchwg.ui
)

add_library(${BIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${SHARED_SOURCES}
    ${SHARED_UI_FILES}
)

# Keep default "lib" prefix — DFMExt expects libdfm-extension-*.so naming
# set_target_properties(${BIN_NAME} PROPERTIES PREFIX "")

# Fail at link time if there are undefined symbols (catches missing source files early)
target_link_options(${BIN_NAME} PRIVATE -Wl,--no-undefined)

target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_part/QJsonModel/include
)

# Link against dfm-extension, Qt (using compat variables), and QJsonModel
target_link_libraries(${BIN_NAME}
    PRIVATE
    ${QT_CORE_TARGET}
    ${QT_GUI_TARGET}
    ${QT_WIDGETS_TARGET}
    ${QT_CONCURRENT_TARGET}
    ${dfm-extension_LIBRARIES}
    QJsonModel
)

target_compile_definitions(${BIN_NAME} PRIVATE
    ${QT_COMPAT_DEFINES}
)

# Installation
include(GNUInstallDirs)
if(NOT DEFINED DFM_EXT_PLUGIN_DIR)
    set(DFM_EXT_PLUGIN_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/dde-file-manager/plugins/extensions)
endif()

install(TARGETS ${BIN_NAME} LIBRARY DESTINATION ${DFM_EXT_PLUGIN_DIR})
