# SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
# SPDX-License-Identifier: MIT

# Extension sources
set(ZCONFIGURE_GUI_SOURCES
    main.cpp
    zconfiguremainwindow.cpp
    configurebuildwidget.cpp
    zconfigureconfig.cpp
)

# Dependencies from src/ (transitive deps of ConfigureBuildWidget)
set(ZCONFIGURE_GUI_DEPS
    ${CMAKE_SOURCE_DIR}/src/widgets/tabconfigwg.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/tablefmtwg.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/basefmtwg.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/searchwg.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/progressdlg.cpp
    ${CMAKE_SOURCE_DIR}/src/widgets/helpquerywg.cpp
    ${CMAKE_SOURCE_DIR}/src/model/mediainfotabelmodel.cpp
    ${CMAKE_SOURCE_DIR}/src/model/multicolumnsearchproxymodel.cpp
    ${CMAKE_SOURCE_DIR}/src/common/common.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zcommandexecutor.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zjsonconfig.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ztexteditor.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zsyntaxhighlighter.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ztexthighlighter.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zflowlayout.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zwindowhelper.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ztableheadermanager.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zmultiselectmenu.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zffprobe.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zffmpeg.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zffplay.cpp
    ${CMAKE_SOURCE_DIR}/src/common/zfftoolconfig.cpp
    ${CMAKE_SOURCE_DIR}/src/component/zprogressbar.cpp
)

add_executable(zconfigure-gui ${ZCONFIGURE_GUI_SOURCES} ${ZCONFIGURE_GUI_DEPS})

target_include_directories(zconfigure-gui PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_part/QJsonModel/include
)

target_link_libraries(zconfigure-gui
    QJsonModel
    ${QT_CORE_TARGET}
    ${QT_WIDGETS_TARGET}
    ${QT_GUI_TARGET}
    ${QT_CONCURRENT_TARGET}
)

target_compile_definitions(zconfigure-gui PRIVATE
    ${QT_COMPAT_DEFINES}
)

# Translation files
set(ZCONFIGURE_GUI_TRANSLATIONS
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/zconfigure-gui_zh_CN.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/zconfigure-gui_en_US.ts
)

# lupdate target: extract translatable strings from sources
# This scans both extension sources AND widget dependencies
if(QT_MAJOR_VERSION EQUAL 5)
    find_package(Qt5 REQUIRED COMPONENTS LinguistTools)
    set(LUPDATE_EXECUTABLE "/usr/lib/qt5/bin/lupdate")
else()
    set(LUPDATE_EXECUTABLE "/usr/lib/qt6/bin/lupdate")
endif()

if(EXISTS "${LUPDATE_EXECUTABLE}")
    # Combine all source files for lupdate scanning
    set(LUPDATE_SOURCES ${ZCONFIGURE_GUI_SOURCES} ${ZCONFIGURE_GUI_DEPS})

    add_custom_target(zconfigure-gui_lupdate
        COMMAND ${LUPDATE_EXECUTABLE}
            ${LUPDATE_SOURCES}
            -ts ${ZCONFIGURE_GUI_TRANSLATIONS}
        COMMENT "Updating zconfigure-gui translation files"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        VERBATIM
    )
    message(STATUS "Added 'zconfigure-gui_lupdate' target to extract translations")
endif()

# Generate .qm files (Qt5/Qt6 compatible)
if(QT_MAJOR_VERSION EQUAL 5)
    find_package(Qt5 REQUIRED COMPONENTS LinguistTools)
    QT5_ADD_TRANSLATION(ZCONFIGURE_GUI_QM_FILES ${ZCONFIGURE_GUI_TRANSLATIONS})
    target_sources(zconfigure-gui PRIVATE ${ZCONFIGURE_GUI_QM_FILES})
else()
    set(ZCONFIGURE_GUI_QM_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/zconfigure-gui_zh_CN.qm
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/zconfigure-gui_en_US.qm
    )
    set(LRELEASE_EXECUTABLE "/usr/lib/qt6/bin/lrelease")
    if(EXISTS "${LRELEASE_EXECUTABLE}")
        add_custom_target(zconfigure-gui_lrelease ALL
            COMMAND ${LRELEASE_EXECUTABLE} ${ZCONFIGURE_GUI_TRANSLATIONS}
            COMMENT "Releasing zconfigure-gui translation files"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            BYPRODUCTS ${ZCONFIGURE_GUI_QM_FILES}
        )
        add_dependencies(zconfigure-gui zconfigure-gui_lrelease)
    endif()
endif()

# Copy translations to build directory
add_custom_command(TARGET zconfigure-gui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/translations
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/zconfigure-gui_zh_CN.qm
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/zconfigure-gui_en_US.qm
        ${CMAKE_BINARY_DIR}/bin/translations/
    COMMENT "Copying zconfigure-gui translation files"
)

# Install
install(TARGETS zconfigure-gui
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/zconfigure-gui_zh_CN.qm
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/zconfigure-gui_en_US.qm
    DESTINATION share/zconfigure-gui/translations
)

# Install desktop file
install(FILES
    ${CMAKE_SOURCE_DIR}/assets/zconfigure-gui.desktop
    DESTINATION share/applications
)

# Install icon
install(FILES
    ${CMAKE_SOURCE_DIR}/assets/128x128/zconfigure-gui.svg
    DESTINATION share/icons/hicolor/scalable/apps
)

# Install man page
install(FILES
    ${CMAKE_SOURCE_DIR}/man/zconfigure-gui.1
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/man/man1
)
