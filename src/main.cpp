/*
 * dfm-menu-manager - DFM right-click menu manager
 * Copyright (C) 2025 zhanghongyuan <zhanghongyuan@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include "core/menu_manager.h"
#include "core/file_type_manager.h"
#include "models/menu_tree_model.h"
#include "models/menu_file_model.h"
#include "utils/window_manager.h"

int main(int argc, char *argv[])
{
    qDebug() << "=== DFM Menu Manager Starting ===";
    
    // 设置高DPI属性(必须在创建QGuiApplication之前)
    qDebug() << "Setting High DPI attributes...";
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    
    qDebug() << "Creating QGuiApplication...";
    QGuiApplication app(argc, argv);
    app.setApplicationName("dfm-menu-manager");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("deepin");
    qDebug() << "QGuiApplication created successfully";
    
    // 加载翻译
    qDebug() << "Loading translations...";
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "dfm-menu-manager_" + QLocale(locale).name();
        qDebug() << "Trying to load translation:" << baseName;
        
        // 尝试从多个路径加载翻译文件
        QStringList searchPaths;
        searchPaths << ":/i18n/translations/" 
                    << QCoreApplication::applicationDirPath() + "/translations/"
                    << QCoreApplication::applicationDirPath() + "/../share/dfm-menu-manager/translations/";
        
        bool loaded = false;
        for (const QString &path : searchPaths) {
            qDebug() << "  Trying path:" << path + baseName + ".qm";
            if (translator.load(path + baseName + ".qm")) {
                app.installTranslator(&translator);
                qDebug() << "Successfully loaded translation from:" << path + baseName + ".qm";
                loaded = true;
                break;
            }
        }
        
        if (loaded) {
            break;
        }
    }
    
    // 注册QML类型
    qDebug() << "Registering QML types...";
    qmlRegisterType<MenuTreeModel>("DFMMenu", 1, 0, "MenuTreeModel");
    qmlRegisterType<MenuFileModel>("DFMMenu", 1, 0, "MenuFileModel");
    qmlRegisterType<MenuManager>("DFMMenu", 1, 0, "MenuManager");
    qmlRegisterType<FileTypeManager>("DFMMenu", 1, 0, "FileTypeManager");
    qmlRegisterSingletonType<WindowManager>("DFMMenu", 1, 0, "WindowManager",
        [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject* {
            Q_UNUSED(scriptEngine)
            WindowManager* instance = WindowManager::instance();
            // 设置对象所有权为 C++，防止 QML 引擎删除它
            QQmlEngine::setObjectOwnership(instance, QQmlEngine::CppOwnership);
            return instance;
        }
    );
    qDebug() << "QML types registered";
    
    // 创建管理器
    qDebug() << "Creating MenuManager...";
    MenuManager menuManager;
    qDebug() << "Loading configurations...";
    menuManager.loadConfigurations();
    qDebug() << "Configurations loaded";
    
    // 创建文件类型管理器
    qDebug() << "Creating FileTypeManager...";
    FileTypeManager fileTypeManager;
    qDebug() << "FileTypeManager created";
    
    // QML引擎
    qDebug() << "Creating QML engine...";
    QQmlApplicationEngine engine;
    
    // 添加qmldir路径
    qDebug() << "Adding import paths...";
    engine.addImportPath(":/qml/styles");
    
    // 暴露管理器到QML
    qDebug() << "Setting context properties...";
    engine.rootContext()->setContextProperty("menuManager", &menuManager);
    engine.rootContext()->setContextProperty("fileTypeManager", &fileTypeManager);
    
    // 加载主QML文件
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    qDebug() << "Loading QML file:" << url;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        qDebug() << "QML object created:" << obj << "URL:" << objUrl;
        if (!obj && url == objUrl) {
            qWarning() << "Failed to create QML object, exiting...";
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    
    engine.load(url);
    qDebug() << "QML load initiated";
    
    qDebug() << "=== Entering event loop ===";
    return app.exec();
}

