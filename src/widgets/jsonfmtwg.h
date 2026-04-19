// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef JSONFMTWG_H
#define JSONFMTWG_H

#include <QWidget>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QClipboard>
#include <QApplication>

#include <widgets/basefmtwg.h>
#include <widgets/searchwg.h>

#include <common/qtcompat.h>

#include <QJsonModel.hpp>

namespace Ui {
class JsonFormatWG;
}

class JsonFormatWG : public BaseFormatWG
{
    Q_OBJECT

public:
    explicit JsonFormatWG(QWidget *parent = nullptr);
    ~JsonFormatWG();

    void setControlMargin(const int &top, const int &bottom, const int &left, const int &right);

    // Edit mode control
    void setEditMode(bool enabled);
    bool isEditMode() const;

    // Config save path for edit mode
    void setConfigSavePath(const QString &group);
    QString configSavePath() const;

signals:
    void dataChanged();
    void editModeChanged(bool enabled);

protected:
    // Override close event to check for unsaved default values
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::JsonFormatWG *ui;

    QJsonModel * m_model;
    QSortFilterProxyModel * m_proxyModel;
    SearchWG * m_searchWG;

protected:
    // Implement pure virtual functions from base class
    void initUI() override;
    void initMenu() override;
    void initConnection() override;
    void initShortCut() override;
    void initExtra() override;
    
    bool loadJson(const QByteArray &json) override;
    
    // Override base class virtual functions
    void showContextMenu(const QPoint &pos) override;
    
    // Override base class information retrieval functions
    QString getKeyForIndex(const QModelIndex &index) override;
    QString getValueForIndex(const QModelIndex &index) override;
    QModelIndexList getSelectedIndexes() override;
    bool hasValidSelection() override;
    QString getAllData() override;

private slots:
    void on_searchReady();
    void on_searchTextChanged(const QString &text);
    void on_searchClear();

    void copyValue();
    void copyKeyValue();
    void copyKey();
    void expand();
    void expandAll();
    void collapse();
    void collapseAll();
    void toggleSearch();
    void toggleSwitchView();

    // Edit mode slots
    void toggleEditMode();
    void insertItem();
    void insertChildItem();
    void deleteItem();
    void onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

private:
    QMenu *m_copyMenu = nullptr;
    QMenu *m_expandMenu = nullptr;
    QMenu *m_collapseMenu = nullptr;

    QAction *m_searchAction = nullptr;
    QAction *m_switchViewAction = nullptr;
    QShortcut *m_searchShortcut = nullptr;

    // Edit mode actions and menus
    QAction *m_editModeAction = nullptr;
    QAction *m_insertAction = nullptr;
    QAction *m_insertChildAction = nullptr;
    QAction *m_deleteAction = nullptr;
    QMenu *m_insertMenu = nullptr;

    // Edit mode state
    bool m_editMode = false;
    QString m_configSavePath;
    QString m_defaultInsertKey;
    QString m_defaultInsertValue;

    void updateEditModeUI();
    void saveConfig();
    bool checkDefaultValues();

private:
    void countVisibleAndTotalItems(QAbstractItemModel *model, const QModelIndex &parent, int &visibleCount, int &totalCount);
    QJsonTreeItem* getItemForIndex(const QModelIndex &index);

};

#endif // JSONFMTWG_H
