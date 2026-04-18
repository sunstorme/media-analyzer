// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef STREAMSWG_H
#define STREAMSWG_H

#include <QWidget>

#include <common/zsingleton.h>
#include <widgets/jsonfmtwg.h>

namespace Ui {
class StreamsWG;
}

class StreamsWG : public QWidget
{
    Q_OBJECT

public:
    DECLARE_ZSINGLETON(StreamsWG)
    StreamsWG(QWidget *parent = nullptr);
    ~StreamsWG();

public:
    void setStreamsData(const QByteArray &data);
    JsonFormatWG* streamsWidget() const;

private:
    Ui::StreamsWG *ui;
    JsonFormatWG *m_streamsWidget = nullptr;
};

#endif // STREAMSWG_H
