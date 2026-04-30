// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "mediainfodisplayer.h"
#include "zffprobe.h"
#include "zwindowhelper.h"
#include "zlogger.h"
#include "dfm-plugin/i18n.h"

#include "widgets/tablefmtwg.h"
#include "widgets/jsonfmtwg.h"
#include "widgets/progressdlg.h"

#include <QtConcurrent>
#include <QMetaObject>
#include <QFile>
#include <QApplication>

void MediaInfoDisplayer::showMediaInfo(const QString &filePath,
                                         const QString &ffprobeFunction,
                                         const QString &windowTitle,
                                         const ZExtraInfo &extrainfo)
{
    if (filePath.isEmpty() || !QFile::exists(filePath)) {
        qWarning() << "File does not exist:" << filePath;
        return;
    }

    bool isStreamingCompatible = (extrainfo.formatKey == FORMAT_TABLE) &&
                                  (ffprobeFunction.contains("show_frames") || ffprobeFunction.contains("show_packets"));

    if (isStreamingCompatible) {
        TableFormatWG *tableWindow = new TableFormatWG;
        tableWindow->setAttribute(Qt::WA_DeleteOnClose);
        tableWindow->setWindowTitle(windowTitle);
        tableWindow->setExtraInfo(extrainfo);
        tableWindow->show();
        tableWindow->raise();
        tableWindow->activateWindow();
        ZWindowHelper::centerToParent(tableWindow);

        QString arrayKey = ffprobeFunction.contains("show_packets") ? "packets" : "frames";

        QStringList args;
        args << ffmpegCommandList
             << ffprobeFunction.split(" ", QT_SKIP_EMPTY_PARTS)
             << OF << JSON << FI << filePath;

        tableWindow->startStreamingLoad(FFPROBE, args, arrayKey);

        bool isPackets = ffprobeFunction.contains("show_packets");
        tableWindow->startTotalCountQuery(filePath, isPackets);

        qDebug() << "Streaming media info:" << FFPROBE << args.join(" ");
    } else {
        ProgressDialog *progressDlg = new ProgressDialog;
        progressDlg->setWindowTitle(PLUGIN_TR("Parsing Media: %1").arg(filePath));
        progressDlg->setProgressMode(ProgressDialog::Indeterminate);
        progressDlg->setMessage(PLUGIN_TR("Parsing"));
        progressDlg->setAutoClose(true);

        progressDlg->start();
        progressDlg->raise();
        progressDlg->activateWindow();
        QtConcurrent::run([=]() {
            ZFfprobe probe;
            QString formats = probe.getMediaInfoJsonFormat(ffprobeFunction, filePath);

            // Must create/show widgets on the main thread
            QMetaObject::invokeMethod(qApp, [=]() {
                popMediaInfoWindow(windowTitle, formats, extrainfo);
            });

            qDebug() << "Media info query completed";
            emit progressDlg->messageChanged(PLUGIN_TR("Finished"));
            emit progressDlg->toFinish();
            QMetaObject::invokeMethod(progressDlg, "deleteLater");
        });
        progressDlg->exec();
    }
}

void MediaInfoDisplayer::popMediaInfoWindow(const QString &title,
                                              const QString &info,
                                              const ZExtraInfo &extrainfo)
{
    BaseFormatWG *mediaInfoWindow = nullptr;
    if (extrainfo.formatKey == FORMAT_JSON) {
        mediaInfoWindow = new JsonFormatWG;
    } else if (extrainfo.formatKey == FORMAT_TABLE) {
        mediaInfoWindow = new TableFormatWG;
    }

    if (!mediaInfoWindow)
        return;

    mediaInfoWindow->setExtraInfo(extrainfo);
    mediaInfoWindow->setWindowTitle(title);
    mediaInfoWindow->setAttribute(Qt::WA_DeleteOnClose);
    mediaInfoWindow->show();
    mediaInfoWindow->raise();
    mediaInfoWindow->activateWindow();
    ZWindowHelper::centerToParent(mediaInfoWindow);
    mediaInfoWindow->loadJson(info.toUtf8());

    qDebug() << title << info.size();
}

void MediaInfoDisplayer::showFormatInfo(const QString &filePath)
{
    ZExtraInfo extrainfo(SHOW_FORMAT, FORMAT_JSON);
    extrainfo.commandList << FFPROBE << ffmpegCommandList << SHOW_FORMAT << OF << JSON << FI << filePath;

    showMediaInfo(filePath, SHOW_FORMAT,
                  QString("[%1] - %2").arg(filePath).arg(extrainfo.commandList.join(" ")),
                  extrainfo);
}

void MediaInfoDisplayer::showStreamInfo(const QString &filePath)
{
    ZExtraInfo extrainfo(SHOW_STREAMS, FORMAT_JSON);
    extrainfo.commandList << FFPROBE << ffmpegCommandList << SHOW_STREAMS << OF << JSON << FI << filePath;

    showMediaInfo(filePath, SHOW_STREAMS,
                  QString("[%1] - %2").arg(filePath).arg(extrainfo.commandList.join(" ")),
                  extrainfo);
}

void MediaInfoDisplayer::showFrameInfo(const QString &filePath, int streamIndex)
{
    QString command = QString("%1 %2 %3").arg(SHOW_FRAMES).arg(SELECT_STREAMS).arg(streamIndex);
    ZExtraInfo extrainfo(command, FORMAT_TABLE);
    extrainfo.commandList << FFPROBE << ffmpegCommandList << command << OF << JSON << FI << filePath;

    showMediaInfo(filePath, command,
                  QString("[%1] - %2").arg(filePath).arg(extrainfo.commandList.join(" ")),
                  extrainfo);
}

void MediaInfoDisplayer::showPacketInfo(const QString &filePath, int streamIndex)
{
    QString command = QString("%1 %2 %3").arg(SHOW_PACKETS).arg(SELECT_STREAMS).arg(streamIndex);
    ZExtraInfo extrainfo(command, FORMAT_TABLE);
    extrainfo.commandList << FFPROBE << ffmpegCommandList << command << OF << JSON << FI << filePath;

    showMediaInfo(filePath, command,
                  QString("[%1] - %2").arg(filePath).arg(extrainfo.commandList.join(" ")),
                  extrainfo);
}
