// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef FILESWG_H
#define FILESWG_H

#include <QWidget>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QMenu>
#include <QAction>
#include <QPair>
#include <QListView>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QMimeData>

#include <common/zsingleton.h>
#include <model/fileshistorymodel.h>

namespace Ui {
class FilesWG;
}

class FilesWG : public QWidget
{
    Q_OBJECT

public:
    DECLARE_ZSINGLETON(FilesWG)
    explicit FilesWG(QWidget *parent = nullptr);
    ~FilesWG();
    
public:
    void addActions(const QList<QAction*> actions);
    void addSubActions(const QString& menu, const QList<QAction*>& actions);
    void addMenus(const QList<QMenu*> menus);
    void addSeparator();

signals:
    void currentFileActived(QPair<QString, QString> filePair);

public slots:
    // Method to add a file to the history, can be called from other parts of the application
    void addFileToHistory(const QString &filePath);

    QString getCurrentSelectFileName();

    void onListViewDoubleClicked(const QModelIndex &index);
    void onListViewActivated(const QModelIndex &index);
    void onCustomContextMenuRequested(const QPoint &pos);

private:
    Ui::FilesWG *ui;
    FilesHistoryModel *m_model = nullptr;
    QMenu *m_contextMenu = nullptr;
    QAction *m_removeAction = nullptr;
    QAction *m_deleteFileAction = nullptr;
    QAction *m_openLocationAction = nullptr;
    QAction *m_copyPathAction = nullptr;
    QAction *m_playAction = nullptr;

    void createContextMenu();
    void onFilesDropped(const QStringList &filePaths);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

#endif // FILESWG_H
