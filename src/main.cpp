// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#endif

#include "common/common.h"
#include "common/zlogger.h"
#include "common/zffprobe.h"

/**
 * @brief logConfig
 * @param app
 *
 * Log configuration
 */
void logConfig(const QApplication& app);

/**
 * @brief commandConfig
 * @param app
 *
 * CommandLine configuration
 */
int commandConfig(const QApplication& app);

void translateConfig(const QApplication& app);

int main(int argc, char *argv[])
{
    // Set UTF-8 encoding for Qt5 to handle Chinese paths correctly in debug mode
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif

    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(APPLICATION_VERSION);

    QApplication app(argc, argv);

    translateConfig(app);

    return commandConfig(app);
}

void logConfig(const QApplication &app) {
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    ZLogger::instance()->loadConfig(settings);
    ZLogger::instance()->setConfigValue(LoggerConfig::DIRECTORY_KEY, "app_logs");
    ZLogger::instance()->setConfigValue(LoggerConfig::MAX_FILE_SIZE_KEY, 5);
    ZLogger::instance()->setConfigValue(LoggerConfig::MAX_FILES_KEY, 10);
    ZLogger::instance()->setConfigValue(LoggerConfig::LEVEL_KEY, static_cast<int>(LogLevel::LOG_INFO));

    QString fileName = "myapp_%1.log";
    ZLogger::instance()->setConfigValue(LoggerConfig::FILE_NAME_PATTERN_KEY, fileName);

    if (ZLogger::instance()->initializeWithConfig()) {
        qInfo() << "Logger initialized successfully with configuration";
    }

    ZLogger::instance()->saveConfig(settings);
    settings.sync();

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [] {
        ZLogger::instance()->shutdown();
    });
}

int commandConfig(const QApplication& app) {
    QCommandLineParser parser;

    parser.setApplicationDescription("Media Analyzer - Media file debugging tool");

    // options
    QCommandLineOption mediaInfoOption(QStringList() << "m" << "media-info",
                                       "Query media file audio and video stream information",
                                       "file");

    QCommandLineOption mediaInfoStreamTypeOption(QStringList() << "s" << "stream-type",
                                                 "Set available stream type (audio, video, a, v) ",
                                                 "type");

    QCommandLineOption mediaInfoFrameTypeOption(QStringList() << "f" << "frame-type",
                                                "Set available frame or packet type (frame, packet, f, p) ",
                                                "type");

    QCommandLineOption mediaInfoStreamsOption(QStringList() << "stms" << "streams",
                                              "media file streams info");

    QCommandLineOption mediaInfoFormatOption(QStringList() << "fmt" << "format",
                                             "media file format info");

    QCommandLineOption basicInfoOption(QStringList() << "b" << "basic-info",
                                       "Query basic info (version, buildconf, formats, muxers, demuxers, devices, codecs, "
                                       "decoders, encoders, bsfs, protocols, filters, pixfmts, layouts, samplefmts, colors, license)",
                                       "type");

    QCommandLineOption cliOption(QStringList() << "c" << "cli",
                                 "No graphical user interface is displayed; the system operates exclusively in command-line mode.");

    QCommandLineOption mainOption(QStringList() << "d" << "detail-window",
                                  "show mainwindow");

    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(basicInfoOption);
    parser.addOption(mediaInfoOption);
    parser.addOption(mediaInfoStreamTypeOption);
    parser.addOption(mediaInfoFrameTypeOption);
    parser.addOption(mediaInfoStreamsOption);
    parser.addOption(mediaInfoFormatOption);
    parser.addOption(cliOption);
    parser.addOption(mainOption);

    parser.process(app);

    QString streamType, frameType, filePath, mediaCmd;

    // media info
    if (parser.isSet(mediaInfoOption)) {
        filePath = parser.value(mediaInfoOption);

        if (!QFileInfo(filePath).exists()) {
            qDebug() << "Error: Please specify the path of the media file" << filePath;
            parser.showHelp();
            return 1;
        }

        if (parser.isSet(mediaInfoStreamTypeOption) && parser.isSet(mediaInfoFrameTypeOption)) {
            streamType = parser.value(mediaInfoStreamTypeOption);
            frameType = parser.value(mediaInfoFrameTypeOption);

            if (!QStringList{"audio", "video", "a", "v"}.contains(streamType)) {
                qDebug() << "Error: Please specify the frame type, (audio, video, a, v)";
                parser.showHelp();
                return 1;
            }
            streamType = streamType.left(1);

            if (!QStringList{"frame", "packet", "f", "p"}.contains(frameType)) {
                qDebug() << "Error: Please specify the frame type, (frame, packet, f, p)";
                parser.showHelp();
                return 1;
            }
            if (frameType.startsWith("f")) {
                frameType = SHOW_FRAMES;
            } else {
                frameType = SHOW_PACKETS;
            }

            mediaCmd = QString("%1 -select_streams %2:0").arg(frameType).arg(streamType);
        }

        if (parser.isSet(mediaInfoStreamsOption)) {
            mediaCmd.append(SHOW_STREAMS + QString(" "));
        }

        if (parser.isSet(mediaInfoFormatOption)) {
            mediaCmd.append(SHOW_FORMAT + QString(" "));
        }

        // cli options
        if (parser.isSet(cliOption)) {
            ZFfprobe ffprobe;

            QString mediaInfo = ffprobe.getMediaInfoJsonFormat(mediaCmd, filePath);
            if (mediaInfo.isEmpty()) {
                qDebug() << "Error: Media file information cannot be obtained. "
                            "Please check if the file path is correct";
                parser.showHelp();
                return 1;
            }

            printf("\nMedai Info:\n%s", mediaInfo.toUtf8().data());
            return 0;
        }
    }

    // basic info
    if (parser.isSet(basicInfoOption) && parser.isSet(cliOption)) {
        ZFfprobe ffprobe;

        QString function = parser.value(basicInfoOption).toLower();
        QString basicinfo;

        bool sucess = false;

        basicinfo = ffprobe.getBasicInfo(function, &sucess);

        if (!sucess) {
            qDebug() << "Error: Unsupported basic information type. The supported types include: \n" <<
                "version, buildconf, formats, muxers, demuxers, devices, codecs, decoders, encoders, bsfs, protocols, "
                "filters, pixfmts, layouts, samplefmts, colors, license";

            parser.showHelp();
            return 1;
        }

        printf("\nBasic Info:\n%s", basicinfo.toUtf8().data());
        return 0;
    }

    logConfig(app);

    // Get the singleton instance
    Common* common = Common::instance();

    // Initialize if not already initialized
    if (!common->isInitialized()) {
        common->initialize();
    }

    MainWindow w;
    ZExtraInfo info;

    if (parser.isSet(mediaInfoOption) &&
        parser.isSet(mediaInfoStreamTypeOption) &&
        parser.isSet(mediaInfoFrameTypeOption)) {
        QString filePath = parser.value(mediaInfoOption);
        info.commandKey = mediaCmd;
        info.commandList << FFPROBE << ffmpegCommandList << mediaCmd << OF << JSON << FI << filePath;
        info.formatKey = FORMAT_TABLE;

        w.showMediaInfo(filePath, mediaCmd, QString("[%1] - %2").arg(filePath).arg(info.commandList.join(" ")), info);
    }

    if (parser.isSet(mediaInfoFormatOption) || parser.isSet(mediaInfoStreamsOption)) {
        QString filePath = parser.value(mediaInfoOption);
        info.commandKey = mediaCmd;
        info.commandList << FFPROBE << ffmpegCommandList << mediaCmd << OF << JSON << FI << filePath;
        info.formatKey = FORMAT_JSON;

        w.showMediaInfo(filePath, mediaCmd, info.commandList.join(" "), info);
    }

    if (parser.isSet(basicInfoOption)) {
        QString function = parser.value(basicInfoOption);

        info.commandKey = function;
        info.commandList << FFPROBE << ffmpegCommandList << function.toLower().prepend("-");
        info.formatKey = FORMAT_TABLE;

        w.showBasicInfo(function, info.commandList.join(" "), info);
    }

    if (parser.isSet(cliOption) || parser.isSet(mediaInfoOption) || parser.isSet(basicInfoOption)) {
        w.hide();
    } else {
        w.show();
    }

    return app.exec();

}

void translateConfig(const QApplication& app) {
    static QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    
    QStringList searchPaths;
    searchPaths << QApplication::applicationDirPath() + "/translations";
    searchPaths << QApplication::applicationDirPath() + "/../share/media-analyzer/translations";
    searchPaths << "/usr/local/share/media-analyzer/translations";
    searchPaths << "/usr/share/media-analyzer/translations";
    searchPaths << ":/translations";
    
    for (const QString &locale : uiLanguages) {
        const QString baseName = "media-analyzer_" + QLocale(locale).name();
        
        for (const QString &path : searchPaths) {
            QString fullPath = path + "/" + baseName + ".qm";
            qDebug() << "Trying to load translation:" << fullPath;
            if (translator.load(fullPath)) {
                const_cast<QApplication&>(app).installTranslator(&translator);
                qDebug() << "Loaded translation:" << fullPath;
                return;
            }
        }
    }
    
    qDebug() << "No translation loaded for locale:" << QLocale::system().name();
}
