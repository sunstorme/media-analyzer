// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef BASEFMTWG_H
#define BASEFMTWG_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QApplication>
#include <QModelIndexList>

#include "common/common.h"

class BaseFormatWG : public QWidget
{
    Q_OBJECT
public:
    explicit BaseFormatWG(QWidget *parent = nullptr);

public:
    bool loadData(const QByteArray& jsonData);
    void setExtraInfo(const ZExtraInfo &info);
    ZExtraInfo getExtraInfo();

    // Context menu extension interface
    void addCustomMenuActions(QMenu *contextMenu);
    void addContextMenuSeparator();
    void addContextMenuAction(QAction *action);
    void addContextMenuMenu(QMenu *menu);
    void removeContextMenuAction(QAction *action);
    void removeContextMenuMenu(QMenu *menu);

protected:
    // Template method - defines initialization process
    void initialize();
    
    // Virtual functions that subclasses need to override
    virtual void initUI() = 0;
    virtual void initMenu() = 0;
    virtual void initConnection() = 0;
    virtual void initShortCut() = 0;
    virtual void initExtra() {}  // Optional additional initialization steps

    void createBaseContextMenu();

    virtual void showContextMenu(const QPoint &pos);

    // Virtual functions for getting selected item information, need to be implemented by subclasses
    virtual QString getKeyForIndex(const QModelIndex &index) { Q_UNUSED(index); return QString(); }
    virtual QString getValueForIndex(const QModelIndex &index) { Q_UNUSED(index); return QString(); }
    virtual QModelIndexList getSelectedIndexes() { return QModelIndexList(); }
    virtual bool hasValidSelection() { return false; }
    
    // Virtual function for getting all data, need to be implemented by subclasses
    virtual QString getAllData() { return QString(); }

signals:

public:
    virtual bool loadJson(const QByteArray &json) = 0;

protected:
    QMenu *m_contextMenu;

private:
    ZExtraInfo m_extraInfo;
};

#endif // BASEFMTWG_H
