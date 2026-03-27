cmake_minimum_required(VERSION 3.16)
project(dfm-menu-manager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt6 依赖
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    Qml
    LinguistTools
)

# 设置源文件
set(SOURCES
    src/main.cpp
    src/core/menu_manager.cpp
    src/core/config_parser.cpp
    src/core/config_writer.cpp
    src/core/file_watcher.cpp
    src/core/file_type_manager.cpp
    src/models/menu_tree_model.cpp
    src/models/menu_file_model.cpp
    src/models/menu_action_item.cpp
    src/utils/config_helper.cpp
    src/utils/file_utils.cpp
    src/utils/window_manager.cpp
)

# 设置头文件
set(HEADERS
    src/core/menu_manager.h
    src/core/config_parser.h
    src/core/config_writer.h
    src/core/file_watcher.h
    src/core/file_type_manager.h
    src/models/menu_tree_model.h
    src/models/menu_file_model.h
    src/models/menu_action_item.h
    src/utils/config_helper.h
    src/utils/file_utils.h
    src/utils/window_manager.h
)

# 设置QML资源文件
set(QML_RESOURCES
    src/resources/qml.qrc
    src/resources/i18n.qrc
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${QML_RESOURCES}
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
)

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 翻译文件
set(TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/dfm-menu-manager_zh_CN.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/translations/dfm-menu-manager_en_US.ts
)

# 添加自定义目标用于更新翻译
add_custom_target(lupdate
    COMMAND Qt6::lupdate ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/qml -ts ${TS_FILES}
    COMMENT "Updating translation files"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(lrelease ALL
    COMMAND Qt6::lrelease ${TS_FILES}
    COMMENT "Releasing translation files"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    BYPRODUCTS
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/dfm-menu-manager_zh_CN.qm
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/dfm-menu-manager_en_US.qm
)

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# 测试
enable_testing()
add_subdirectory(tests)

