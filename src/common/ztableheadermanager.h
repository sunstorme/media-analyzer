// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZTABLEHEADERMANAGER_H
#define ZTABLEHEADERMANAGER_H

#include <QObject>
#include <QHeaderView>
#include <QMenu>
#include <QSettings>
#include <QHash>
#include <QScrollBar>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>

#include "common.h"
#include "zmultiselectmenu.h"

class ZTableHeaderManager : public QObject
{
    Q_OBJECT

public:
    explicit ZTableHeaderManager(QHeaderView *horizontalHeader, QHeaderView *verticalHeader, QObject *parent = nullptr);
    ~ZTableHeaderManager();

    void setObjectName(const QString &name);
    QString objectName() const;

    void enableHeaderContextMenu(bool enable = true);
    void saveState();
    void restoreState();

    void setTotalCountVisible(bool visible);
    void updateTotalCount(int count);

signals:
    void headerToggleVisiable(int index, bool visiable);

private slots:
    void onHeaderContextMenuRequested(const QPoint &pos);
    void toggleColumnVisibility();
    void showAllColumns();

private:
    void setupTotalCountLabel();

private:
    QHeaderView *m_horizontalHeader = nullptr;
    QHeaderView *m_verticalHeader = nullptr;
    QString m_objectName;
    QHash<QAction*, int> m_actionToColumnMap;
    QLabel *m_totalCountLabel = nullptr;
    bool m_showVerticalHeader;
};

#endif // ZTABLEHEADERMANAGER_H
