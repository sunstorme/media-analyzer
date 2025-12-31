// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef FILESHISTORYMODEL_H
#define FILESHISTORYMODEL_H

#include <QAbstractListModel>
#include <QSettings>
#include <QFileInfo>
#include <QDesktopServices>
#include <QApplication>
#include <QClipboard>
#include <QProcess>
#include <QMenu>
#include <QContextMenuEvent>

#include "common/common.h"

class FilesHistoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum FileRoles {
        FilePathRole = Qt::UserRole + 1,
        FileNameRole
    };

    explicit FilesHistoryModel(QObject *parent = nullptr);
    ~FilesHistoryModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Add/remove data:
    bool addFile(const QString &filePath);
    bool removeFile(const QString &filePath);
    bool removeFile(int index);
    void clearHistory();

    // Load/Save settings
    void loadSettings();
    void saveSettings();

    //
    QString currentFile(int index);

    // Context menu actions
    void removeRecord(int index);
    void removeRecords(const QModelIndexList &indexs);
    void deleteFile(int index);
    void openFileLocation(int index);
    void openFileLocations(const QModelIndexList &indexs);
    void copyFilePath(int index);
    void copyFilePaths(QList<int> indexs);
    void copyFilePaths(QModelIndexList indexs);
    void playFile(int index);

private:
    QStringList m_filePaths;
    QSettings m_settings;
    constexpr static auto FILES_KEY = "RecentFiles";
};

#endif // FILESHISTORYMODEL_H
