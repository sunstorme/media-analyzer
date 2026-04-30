// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "mediaactionhandler.h"
#include "common/mediainfodisplayer.h"
#include "common/zffprobe.h"
#include "common/zlogger.h"
#include "i18n.h"

#include <QProcess>
#include <QFileInfo>
#include <QDir>

MediaActionHandler::MediaActionHandler(QObject *parent)
    : QObject(parent)
{
}

void MediaActionHandler::showFormatInfo(const QString &filePath)
{
    MediaInfoDisplayer::showFormatInfo(filePath);
}

void MediaActionHandler::showStreamInfo(const QString &filePath)
{
    MediaInfoDisplayer::showStreamInfo(filePath);
}

void MediaActionHandler::showFrameInfo(const QString &filePath, int streamIndex)
{
    MediaInfoDisplayer::showFrameInfo(filePath, streamIndex);
}

void MediaActionHandler::showPacketInfo(const QString &filePath, int streamIndex)
{
    MediaInfoDisplayer::showPacketInfo(filePath, streamIndex);
}

void MediaActionHandler::showImageInfo(const QString &filePath)
{
    // Use ffprobe to show image format info as JSON
    ZExtraInfo extrainfo(SHOW_STREAMS, FORMAT_JSON);
    extrainfo.commandList << FFPROBE << ffmpegCommandList << SHOW_STREAMS << OF << JSON << FI << filePath;
    MediaInfoDisplayer::showMediaInfo(filePath, SHOW_STREAMS,
                      QString("[%1] - %2").arg(filePath).arg(extrainfo.commandList.join(" ")),
                      extrainfo);
}

void MediaActionHandler::showImageExif(const QString &filePath)
{
    // Use ffprobe to show format info which includes EXIF tags
    ZExtraInfo extrainfo(SHOW_FORMAT, FORMAT_JSON);
    extrainfo.commandList << FFPROBE << ffmpegCommandList << SHOW_FORMAT << OF << JSON << FI << filePath;
    MediaInfoDisplayer::showMediaInfo(filePath, SHOW_FORMAT,
                      QString("[%1] - EXIF Data").arg(filePath),
                      extrainfo);
}

void MediaActionHandler::convertFormat(const QString &filePath, const QString &targetFormat)
{
    Q_UNUSED(targetFormat)

    // TODO: Launch conversion dialog. For now, use ffmpeg for basic conversion.
    QFileInfo info(filePath);
    QString baseName = info.completeBaseName();
    QString outPath = info.absoluteDir().filePath(baseName + "_converted.mp4");

    QStringList args;
    args << "-y" << "-i" << filePath << outPath;

    launchExternalProcess("ffmpeg", args);
}

void MediaActionHandler::convertToGif(const QString &filePath)
{
    QFileInfo info(filePath);
    QString baseName = info.completeBaseName();
    QString outPath = info.absoluteDir().filePath(baseName + ".gif");

    QStringList args;
    args << "-y" << "-i" << filePath
         << "-vf" << "fps=12"
         << "-loop" << "0"
         << outPath;

    launchExternalProcess("ffmpeg", args);
}

void MediaActionHandler::extractStream(const QString &filePath, int streamIndex, const QString &outputFormat)
{
    QFileInfo info(filePath);
    QString baseName = info.completeBaseName();
    QString outPath = info.absoluteDir().filePath(
        baseName + QString("_stream%1.%2").arg(streamIndex).arg(outputFormat));

    QStringList args;
    args << "-y" << "-i" << filePath
         << "-vn"   // no video
         << "-acodec" << "copy";

    if (streamIndex >= 0) {
        args << "-map" << QString("0:a:%1").arg(streamIndex);
    }

    args << outPath;

    launchExternalProcess("ffmpeg", args);
}

void MediaActionHandler::launchExternalProcess(const QString &program, const QStringList &args)
{
    qInfo() << "Launching:" << program << args.join(" ");

    QProcess *process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [process](int exitCode, QProcess::ExitStatus status) {
                if (status == QProcess::CrashExit) {
                    qWarning() << "Process crashed";
                } else if (exitCode != 0) {
                    qWarning() << "Process exited with code" << exitCode;
                } else {
                    qInfo() << "Process completed successfully";
                }
                process->deleteLater();
            });

    process->start(program, args);
}
