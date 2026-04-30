// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef MEDIAINFODISPLAYER_H
#define MEDIAINFODISPLAYER_H

#include <QString>
#include <QStringList>
#include "common/common.h"

class MediaInfoDisplayer
{
public:
    static void showMediaInfo(const QString &filePath,
                               const QString &ffprobeFunction,
                               const QString &windowTitle,
                               const ZExtraInfo &extrainfo);

    static void showFormatInfo(const QString &filePath);
    static void showStreamInfo(const QString &filePath);
    static void showFrameInfo(const QString &filePath, int streamIndex);
    static void showPacketInfo(const QString &filePath, int streamIndex);

private:
    static void popMediaInfoWindow(const QString &title,
                                    const QString &info,
                                    const ZExtraInfo &extrainfo);
};

#endif // MEDIAINFODISPLAYER_H
