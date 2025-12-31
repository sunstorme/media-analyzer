// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef MEDIAPROPSWG_H
#define MEDIAPROPSWG_H

#include <QWidget>
#include <QTabWidget>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtConcurrent>

#include <widgets/jsonfmtwg.h>
#include <widgets/progressdlg.h>
#include <common/zffprobe.h>
#include <common/zsingleton.h>

// Define missing constants
#define STREAMS "streams"
#define CHAPTERS "chapters"

namespace Ui {
class MediaPropsWG;
}

class MediaPropsWG : public QWidget
{
    Q_OBJECT

public:
    DECLARE_ZSINGLETON(MediaPropsWG)
    explicit MediaPropsWG(QWidget *parent = nullptr);
    ~MediaPropsWG();

public:
    void setMediaFile(const QString &fileName);
    void loadMediaInfo();
    void loadFormatInfo();
    void loadStreamsInfo();

    void loadMediaInfoAsync();

private:
    void InitUI();

private:
    Ui::MediaPropsWG *ui;
    
    QString m_mediaFile;
    ZFfprobe m_probe;
    
    JsonFormatWG *m_formatWidget = nullptr;
    JsonFormatWG *m_streamsWidget = nullptr;
};

#endif // MEDIAPROPSWG_H
