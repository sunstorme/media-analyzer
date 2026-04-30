// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef MEDIAACTIONHANDLER_H
#define MEDIAACTIONHANDLER_H

#include <QObject>
#include <QString>

class MediaActionHandler : public QObject
{
    Q_OBJECT

public:
    explicit MediaActionHandler(QObject *parent = nullptr);

    void showFormatInfo(const QString &filePath);
    void showStreamInfo(const QString &filePath);
    void showFrameInfo(const QString &filePath, int streamIndex);
    void showPacketInfo(const QString &filePath, int streamIndex);
    void showImageInfo(const QString &filePath);
    void showImageExif(const QString &filePath);

    void convertFormat(const QString &filePath, const QString &targetFormat);
    void convertToGif(const QString &filePath);
    void extractStream(const QString &filePath, int streamIndex, const QString &outputFormat);

private:
    void launchExternalProcess(const QString &program, const QStringList &args);
};

#endif // MEDIAACTIONHANDLER_H
