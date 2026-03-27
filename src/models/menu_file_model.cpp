// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "menu_file_model.h"
#include "../core/file_watcher.h"
#include "../utils/file_utils.h"
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QTimer>
#include <QDebug>

MenuFileModel::MenuFileModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_showSystemOnly(false)
    , m_fileWatcher(nullptr)
{
    // 自动加载文件列表
    refresh();
    
    // 设置文件监控
    setupFileWatcher();
}

MenuFileModel::~MenuFileModel() {
    cleanupFileWatcher();
}

void MenuFileModel::setupFileWatcher() {
    if (!m_fileWatcher) {
        m_fileWatcher = new FileWatcher(this);
        
        connect(m_fileWatcher, &FileWatcher::fileChanged,
                this, &MenuFileModel::onFileChanged);
        connect(m_fileWatcher, &FileWatcher::directoryChanged,
                this, &MenuFileModel::onDirectoryChanged);
    }
    
    // 监控用户配置目录
    QString userDir = FileUtils::getUserConfigDir();
    if (QDir(userDir).exists()) {
        m_fileWatcher->watchDirectory(userDir);
        qDebug() << "Watching user directory:" << userDir;
    }
    
    // 监控系统配置目录
    QString systemDir = FileUtils::getSystemConfigDir();
    if (QDir(systemDir).exists()) {
        m_fileWatcher->watchDirectory(systemDir);
        qDebug() << "Watching system directory:" << systemDir;
    }
}

void MenuFileModel::cleanupFileWatcher() {
    if (m_fileWatcher) {
        QString userDir = FileUtils::getUserConfigDir();
        QString systemDir = FileUtils::getSystemConfigDir();
        
        m_fileWatcher->unwatchDirectory(userDir);
        m_fileWatcher->unwatchDirectory(systemDir);
    }
}

void MenuFileModel::onFileChanged(const QString &path) {
    qDebug() << "File changed, refreshing model:" << path;
    // 延迟刷新以避免频繁更新
    QTimer::singleShot(100, this, [this]() {
        refresh();
    });
}

void MenuFileModel::onDirectoryChanged(const QString &path) {
    qDebug() << "Directory changed, refreshing model:" << path;
    // 延迟刷新以避免频繁更新
    QTimer::singleShot(100, this, [this]() {
        refresh();
    });
}

int MenuFileModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_files.size();
}

QVariant MenuFileModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_files.size()) {
        return QVariant();
    }
    
    const FileInfo &file = m_files.at(index.row());
    
    switch (role) {
    case FileNameRole:
        return file.name;
    case FilePathRole:
        return file.path;
    case IsSystemRole:
        return file.isSystem;
    case IsModifiedRole:
        return file.isModified;
    case CommentRole:
        return file.comment;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MenuFileModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[FileNameRole] = "fileName";
    roles[FilePathRole] = "filePath";
    roles[IsSystemRole] = "isSystem";
    roles[IsModifiedRole] = "isModified";
    roles[CommentRole] = "comment";
    return roles;
}

void MenuFileModel::refresh() {
    beginResetModel();
    m_allFiles.clear();
    
    // 根据showSystemOnly属性决定加载哪些文件
    if (!m_showSystemOnly) {
        // 加载用户配置文件
        QString userDir = FileUtils::getUserConfigDir();
        QDir userConfigDir(userDir);
        if (userConfigDir.exists()) {
            QStringList userFiles = userConfigDir.entryList(QStringList() << "*.conf", QDir::Files);
            for (const QString &fileName : userFiles) {
                FileInfo info;
                info.name = fileName;
                info.path = userDir + "/" + fileName;
                info.isSystem = false;
                info.isModified = false;
                m_allFiles.append(info);
            }
        }
    }
    
    if (m_showSystemOnly) {
        // 加载系统配置文件
        QString systemDir = FileUtils::getSystemConfigDir();
        QDir systemConfigDir(systemDir);
        if (systemConfigDir.exists()) {
            QStringList systemFiles = systemConfigDir.entryList(QStringList() << "*.conf", QDir::Files);
            for (const QString &fileName : systemFiles) {
                FileInfo info;
                info.name = fileName;
                info.path = systemDir + "/" + fileName;
                info.isSystem = true;
                info.isModified = false;
                m_allFiles.append(info);
            }
        }
    }
    
    // 应用搜索过滤
    applySearchFilter();
    
    endResetModel();
}

void MenuFileModel::setSearchFilter(const QString &filter) {
    if (m_searchFilter != filter) {
        m_searchFilter = filter;
        emit searchFilterChanged();
        applySearchFilter();
    }
}

void MenuFileModel::applySearchFilter() {
    beginResetModel();
    m_files.clear();
    
    if (m_searchFilter.isEmpty()) {
        // 如果搜索过滤为空，显示所有文件
        m_files = m_allFiles;
    } else {
        // 根据搜索过滤文件
        QString filterLower = m_searchFilter.toLower();
        for (const FileInfo &info : m_allFiles) {
            if (info.name.toLower().contains(filterLower)) {
                m_files.append(info);
            }
        }
    }
    
    endResetModel();
}

void MenuFileModel::createFile(const QString &name) {
    Q_UNUSED(name)
    // TODO: 实现创建文件
}

void MenuFileModel::deleteFile(const QString &path) {
    Q_UNUSED(path)
    // TODO: 实现删除文件
}

void MenuFileModel::renameFile(const QString &path, const QString &newName) {
    Q_UNUSED(path)
    Q_UNUSED(newName)
    // TODO: 实现重命名文件
}

QString MenuFileModel::copyFile(const QString &sourcePath, bool toSystem) {
    Q_UNUSED(sourcePath)
    Q_UNUSED(toSystem)
    // TODO: 实现复制文件
    return QString();
}

void MenuFileModel::openFile(const QString &path) {
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        qWarning() << "File does not exist:" << path;
        return;
    }
    
    qDebug() << "Attempting to open file:" << path;
    
    // 使用Qt的QDesktopServices打开文件（跨平台方式）
    QUrl url = QUrl::fromLocalFile(path);
    bool success = QDesktopServices::openUrl(url);
    
    if (success) {
        qDebug() << "Successfully opened file with Qt:" << path;
    } else {
        qWarning() << "Failed to open file with Qt:" << path;
    }
}

void MenuFileModel::openContainingFolder(const QString &path) {
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        qWarning() << "File does not exist:" << path;
        return;
    }
    
    QString folderPath = fileInfo.absolutePath();
    qDebug() << "Attempting to open containing folder:" << folderPath;
    
    // 首先尝试使用dde-file-manager打开文件夹并选中文件（深度定制功能）
    QStringList ddeArgs;
    ddeArgs << "--show-item" << path;
    
    qint64 ddePid = 0;
    bool ddeSuccess = QProcess::startDetached("dde-file-manager", ddeArgs, QString(), &ddePid);
    
    if (ddeSuccess && ddePid > 0) {
        qDebug() << "Successfully started dde-file-manager for:" << path << "PID:" << ddePid;
        return;
    } else {
        qDebug() << "dde-file-manager not available, using Qt to open folder:" << folderPath;
    }
    
    // 如果dde-file-manager不可用，使用Qt的QDesktopServices打开文件夹
    QUrl url = QUrl::fromLocalFile(folderPath);
    bool success = QDesktopServices::openUrl(url);
    
    if (success) {
        qDebug() << "Successfully opened folder with Qt:" << folderPath;
    } else {
        qWarning() << "Failed to open folder with Qt:" << folderPath;
    }
}
