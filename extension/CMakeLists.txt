# SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
# SPDX-License-Identifier: MIT

# Find Qt Network component
if(QT_MAJOR_VERSION EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Network)
    set(QT_NETWORK_TARGET Qt6::Network)
else()
    find_package(Qt5 REQUIRED COMPONENTS Network)
    set(QT_NETWORK_TARGET Qt5::Network)
endif()

# Extension sources
set(ZJSON_GUI_SOURCES
    main.cpp
    zjsonmainwindow.cpp
    commandhistory.cpp
    zjsonnetworkmanager.cpp
)

# Dependencies from src/ (transitive deps of JsonFormatWG)
set(ZJSON_GUI_DEPS
    ${CMAKE_SOURCE_DIR}/src/widgets/jsonfmtwg.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/basefmtwg.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/searchwg.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/insertnodedialog.cpp
    ${CMAKE_SOURCE_DIR}/src/common/common.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zcommandexecutor.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ztexteditor.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zsyntaxhighlighter.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ztexthighlighter.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zflowlayout.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zjsonconfig.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zwindowhelper.cpp
    ${CMAKE_SOURCE_DIR}/src/component/zprogressbar.cpp
)

add_executable(zjson-gui ${ZJSON_GUI_SOURCES} ${ZJSON_GUI_DEPS})

target_include_directories(zjson-gui PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_part/QJsonModel/include
)

target_link_libraries(zjson-gui
    QJsonModel
    ${QT_CORE_TARGET}
    ${QT_WIDGETS_TARGET}
    ${QT_GUI_TARGET}
    ${QT_CONCURRENT_TARGET}
    ${QT_NETWORK_TARGET}
)

target_compile_definitions(zjson-gui PRIVATE
    ${QT_COMPAT_DEFINES}
)

# Translation files
set(ZJSON_GUI_TRANSLATIONS
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/zjson-gui_zh_CN.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/zjson-gui_en_US.ts
)

# Generate .qm files
if(QT_MAJOR_VERSION EQUAL 5)
    find_package(Qt5 REQUIRED COMPONENTS LinguistTools)
    QT5_ADD_TRANSLATION(ZJSON_GUI_QM_FILES ${ZJSON_GUI_TRANSLATIONS})
    target_sources(zjson-gui PRIVATE ${ZJSON_GUI_QM_FILES})
else()
    set(ZJSON_GUI_QM_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/zjson-gui_zh_CN.qm
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/zjson-gui_en_US.qm
    )
    set(LRELEASE_EXECUTABLE "/usr/lib/qt6/bin/lrelease")
    if(EXISTS "${LRELEASE_EXECUTABLE}")
        add_custom_target(zjson-gui_lrelease ALL
            COMMAND ${LRELEASE_EXECUTABLE} ${ZJSON_GUI_TRANSLATIONS}
            COMMENT "Releasing zjson-gui translation files"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            BYPRODUCTS ${ZJSON_GUI_QM_FILES}
        )
        add_dependencies(zjson-gui zjson-gui_lrelease)
    endif()
endif()

# Copy translations to build directory
add_custom_command(TARGET zjson-gui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/translations
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/zjson-gui_zh_CN.qm
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/zjson-gui_en_US.qm
        ${CMAKE_BINARY_DIR}/bin/translations/
    COMMENT "Copying zjson-gui translation files"
)

# Install
install(TARGETS zjson-gui
    RUNTIME DESTINATION bin
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/zjson-gui_zh_CN.qm
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/zjson-gui_en_US.qm
    DESTINATION ${CMAKE_INSTALL_DATADIR}/zjson-gui/translations
)
