// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <QObject>
#include <QString>
#include "config_parser.h"
#include "config_writer.h"
#include "file_watcher.h"
#include "../models/menu_tree_model.h"
#include "../models/menu_file_model.h"

/**
 * @brief 菜单管理器
 * 
 * 负责配置文件的加载、保存和管理,是整个应用的核心控制器
 */
class MenuManager : public QObject {
    Q_OBJECT
    
public:
    explicit MenuManager(QObject *parent = nullptr);
    
    // 配置文件管理
    Q_INVOKABLE void loadConfigurations();
    Q_INVOKABLE bool saveConfiguration(const QString &filePath);
    Q_INVOKABLE bool createNewConfig(const QString &name, bool isSystem = false);
    Q_INVOKABLE bool deleteConfig(const QString &filePath);
    
    // 菜单操作
    Q_INVOKABLE MenuTreeModel* getMenuModel(const QString &configFile);
    Q_INVOKABLE void setCurrentConfig(const QString &configFile);
    Q_INVOKABLE QString getCurrentConfig() const;
    Q_INVOKABLE bool saveCurrentModel();
    
    // 验证
    Q_INVOKABLE bool validateConfig(const QString &filePath);
    Q_INVOKABLE QStringList getValidationErrors();
    
    // 导出 JSON（用于调试）
    Q_INVOKABLE QString exportToJson(const QString &configFile);
    
    // 获取文件模型
    Q_INVOKABLE MenuFileModel* getFileModel();
    
signals:
    void configLoaded(const QString &configFile);
    void configSaved(const QString &configFile);
    void configChanged(const QString &configFile);
    void errorOccurred(const QString &message);
    
private:
    ConfigParser m_parser;
    ConfigWriter m_writer;
    FileWatcher *m_watcher;
    QMap<QString, MenuTreeModel*> m_models;
    QString m_currentConfig;
    MenuFileModel *m_fileModel;
};

#endif // MENUMANAGER_H
