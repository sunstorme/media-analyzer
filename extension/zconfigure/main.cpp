// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zconfiguremainwindow.h"
#include "zconfigureconfig.h"
#include "common/zwindowhelper.h"
#include "common/common.h"

#include <QApplication>
#include <QCommandLineParser>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

    QCoreApplication::setOrganizationName("MediaTools");
    QCoreApplication::setApplicationName("ZConfigureGui");
    QCoreApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);

    // Load translations using common function
    loadAppTranslations(app, "zconfigure-gui");

    QCommandLineParser parser;
    parser.setApplicationDescription("Graphical configure/build tool for autotools projects");
    parser.addHelpOption();
    parser.addVersionOption();

    // Custom window title option
    QCommandLineOption titleOption(QStringList() << "t" << "title",
        QCoreApplication::translate("main", "Set custom window title"),
        QCoreApplication::translate("main", "title"));

    parser.addOption(titleOption);
    parser.addPositionalArgument("project",
        QCoreApplication::translate("main", "Path to the project directory to open"));

    parser.process(app);

    QString customTitle = parser.value(titleOption);
    const QStringList args = parser.positionalArguments();

    // Create main window
    ZConfigureMainWindow window(nullptr, customTitle);
    window.show();

    return app.exec();
}
