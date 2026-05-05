// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zjsonmainwindow.h"
#include "common/zwindowhelper.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QTranslator>
#include <QLocale>
#include <QStandardPaths>
#include <QSettings>
#include <QDir>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif

#include <unistd.h>

// Split command string respecting quotes
QStringList splitCommandString(const QString &commandStr)
{
    QStringList result;
    QString current;
    bool inQuotes = false;
    bool inSingleQuotes = false;

    for (int i = 0; i < commandStr.length(); ++i) {
        QChar c = commandStr[i];

        if (c == '"' && !inSingleQuotes) {
            inQuotes = !inQuotes;
        } else if (c == '\'' && !inQuotes) {
            inSingleQuotes = !inSingleQuotes;
        } else if (c.isSpace() && !inQuotes && !inSingleQuotes) {
            if (!current.isEmpty()) {
                result.append(current);
                current.clear();
            }
        } else {
            current.append(c);
        }
    }

    if (!current.isEmpty()) {
        result.append(current);
    }

    return result;
}

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

    QCoreApplication::setOrganizationName("MediaTools");
    QCoreApplication::setApplicationName("ZJsonGui");
    QCoreApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);

    // Load translations
    QTranslator translator;
    QString locale = QLocale::system().name();
    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/translations",
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/translations",
    };
#if defined(CMAKE_INSTALL_FULL_DATADIR)
    searchPaths.append(CMAKE_INSTALL_FULL_DATADIR "/zjson-gui/translations");
#endif
    for (const auto &path : searchPaths) {
        if (translator.load("zjson-gui_" + locale, path)) {
            app.installTranslator(&translator);
            break;
        }
    }

    QCommandLineParser parser;
    parser.setApplicationDescription("JSON viewer and command executor");
    parser.addHelpOption();
    parser.addVersionOption();

    // Custom window title option
    QCommandLineOption titleOption(QStringList() << "t" << "title",
        QCoreApplication::translate("main", "Set custom window title"),
        QCoreApplication::translate("main", "title"));

    // Command execution option
    QCommandLineOption cmdOption(QStringList() << "c" << "cmd",
        QCoreApplication::translate("main", "Execute command string"),
        QCoreApplication::translate("main", "command"));

    parser.addOption(titleOption);
    parser.addOption(cmdOption);
    parser.addPositionalArgument("source",
        QCoreApplication::translate("main", "JSON file path or HTTP(S) URL to open"));

    parser.process(app);

    QString customTitle = parser.value(titleOption);
    QString commandStr = parser.value(cmdOption);

    const QStringList args = parser.positionalArguments();
    bool hasPipeInput = !isatty(fileno(stdin));

    // Determine window title
    QString windowTitle;
    if (!customTitle.isEmpty()) {
        windowTitle = customTitle;
    } else if (!commandStr.isEmpty()) {
        windowTitle = commandStr;
    }

    // Command execution: show result window and execute command
    if (!commandStr.isEmpty()) {
        auto *result = new ZJsonResultWindow(nullptr, windowTitle);
        result->setAttribute(Qt::WA_DeleteOnClose);
        ZWindowHelper::centerToCurrentScreen(result);
        result->show();

        // Split command string and execute
        QStringList commandArgs = splitCommandString(commandStr);
        if (!commandArgs.isEmpty()) {
            result->startCommand(commandStr);
        }

        return app.exec();
    }

    // Pipe or file/URL argument: show result window only
    if (hasPipeInput || !args.isEmpty()) {
        auto *result = new ZJsonResultWindow(nullptr, windowTitle);
        result->setAttribute(Qt::WA_DeleteOnClose);
        ZWindowHelper::centerToCurrentScreen(result);
        result->show();

        if (!args.isEmpty()) {
            QString source = args.first();
            if (source.startsWith("http://") || source.startsWith("https://")) {
                result->startFetch(QUrl(source));
            } else {
                QFile file(source);
                if (file.open(QIODevice::ReadOnly)) {
                    result->loadJsonData(file.readAll());
                    file.close();
                } else {
                    qWarning() << "Failed to open file:" << source;
                }
            }
        }

        if (hasPipeInput) {
            QFile stdinFile;
            stdinFile.open(stdin, QIODevice::ReadOnly);
            QByteArray pipedData = stdinFile.readAll();
            stdinFile.close();

            if (!pipedData.isEmpty())
                result->loadJsonData(pipedData);
        }

        return app.exec();
    }

    // No input: show launcher window
    ZJsonMainWindow window(nullptr, customTitle);
    window.show();
    return app.exec();
}
