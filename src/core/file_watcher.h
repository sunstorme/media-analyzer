// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QMap>
#include <QSet>

class FileWatcher : public QObject {
    Q_OBJECT
    
public:
    explicit FileWatcher(QObject *parent = nullptr);
    ~FileWatcher();
    
public slots:
    void watchDirectory(const QString &path);
    void unwatchDirectory(const QString &path);
    
signals:
    void fileChanged(const QString &path);
    void directoryChanged(const QString &path);
    
private slots:
    void onFileChanged(const QString &path);
    void onDirectoryChanged(const QString &path);
    
private:
    QFileSystemWatcher *m_watcher;
    QMap<QString, QSet<QString>> m_directoryFiles;  // 目录路径 -> 该目录下的文件集合
    void scanDirectoryFiles(const QString &path);
};

#endif // FILEWATCHER_H
