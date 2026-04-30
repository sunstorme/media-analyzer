// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include <dfm-extension/dfm-extension.h>

#include "mediamenuplugin.h"

#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>
#include <QDir>

static MediaMenuPlugin *mediaMenu { nullptr };
static QTranslator *pluginTranslator { nullptr };

static void loadPluginTranslations()
{
    pluginTranslator = new QTranslator(qApp);

    QString locale = QLocale::system().name();
    QString qmFile = QString("media-analyzer_%1.qm").arg(locale);

    QStringList searchPaths = {
        QStringLiteral("/usr/share/media-analyzer/translations"),
        QDir::currentPath() + QStringLiteral("/translations"),
    };

    for (const auto &path : searchPaths) {
        if (pluginTranslator->load(qmFile, path)) {
            qApp->installTranslator(pluginTranslator);
            qInfo() << "Plugin translations loaded:" << path << "/" << qmFile;
            return;
        }
    }

    qWarning() << "Plugin translations not found for locale:" << locale;
}

extern "C" {

void dfm_extension_initiliaze()
{
    qInfo() << "[MediaPlugin] dfm_extension_initiliaze called, appName:" << qApp->applicationName();
    if (qApp->applicationName() == "dde-file-manager") {
        loadPluginTranslations();
        mediaMenu = new MediaMenuPlugin;
        qInfo() << "[MediaPlugin] plugin created successfully";
    } else {
        qWarning() << "[MediaPlugin] skipped: appName mismatch";
    }
}

void dfm_extension_shutdown()
{
    if (pluginTranslator) {
        qApp->removeTranslator(pluginTranslator);
        delete pluginTranslator;
        pluginTranslator = nullptr;
    }
    delete mediaMenu;
}

DFMEXT::DFMExtMenuPlugin *dfm_extension_menu()
{
    return mediaMenu;
}

}   // extern "C"
