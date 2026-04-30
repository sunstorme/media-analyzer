// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif

#include <unistd.h>

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

    parser.addPositionalArgument("source",
        QCoreApplication::translate("main", "JSON file path or HTTP(S) URL to open"));

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    bool hasPipeInput = !isatty(fileno(stdin));

    // Pipe or file/URL argument: show result window only
    if (hasPipeInput || !args.isEmpty()) {
        auto *result = new ZJsonResultWindow();
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
    ZJsonMainWindow window;
    window.show();
    return app.exec();
}
