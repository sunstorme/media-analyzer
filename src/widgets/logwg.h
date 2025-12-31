// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef LOGWG_H
#define LOGWG_H

#include <QWidget>

#include <common/zsingleton.h>
#include <common/ztableheadermanager.h>
#include <common/ztexthighlighter.h>

#include <widgets/searchwg.h>

class LogModel;

namespace Ui {
class LogWG;
}

class LogWG : public QWidget
{
    Q_OBJECT

public:
    DECLARE_ZSINGLETON(LogWG)
    LogWG(QWidget *parent = nullptr);
    ~LogWG();

public slots:
    void outLog(const QString &log);

private slots:
    void showContextMenu(const QPoint &pos);
    void toggleSearchDetail();
    void toggleView();
    void on_searchReady();

private:
    Ui::LogWG *ui;
    LogModel *m_logModel = nullptr;
    ZTableHeaderManager *m_headerManager = nullptr;

    ZTextHighlighter *m_highLighter = nullptr;
    SearchWG *m_searchWG = nullptr;
};

#endif // LOGWG_H
