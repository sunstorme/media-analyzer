// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MENUTREEMODEL_H
#define MENUTREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QHash>
#include "menu_action_item.h"
#include "../core/config_parser.h"

/**
 * @brief 菜单树形模型
 * 
 * 为QML提供树形数据模型,支持菜单项的增删改查
 */
class MenuTreeModel : public QAbstractItemModel {
    Q_OBJECT
    
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        NameLocalRole,
        IdRole,
        LevelRole,
        HasChildrenRole,
        IsEditableRole,
        IsSystemRole,
        ExecCommandRole,
        MenuTypesRole,
        SupportSuffixRole,
        PositionNumberRole,
        CommentRole,
        CommentLocalRole,
        VersionRole
    };
    
    explicit MenuTreeModel(QObject *parent = nullptr);
    ~MenuTreeModel();
    
    // QAbstractItemModel 接口
    QModelIndex index(int row, int column, 
                     const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    // 菜单操作
    Q_INVOKABLE void addItem(const QModelIndex &parent, const QString &name);
    Q_INVOKABLE void removeItem(const QModelIndex &index);
    Q_INVOKABLE void moveItem(const QModelIndex &index, int direction);
    Q_INVOKABLE void updateItem(const QModelIndex &index, const QString &role, 
                               const QVariant &value);
    Q_INVOKABLE void addSiblingItem(const QModelIndex &index, const QString &name);
    Q_INVOKABLE void addChildItem(const QModelIndex &index, const QString &name);
    Q_INVOKABLE void renameItem(const QModelIndex &index, const QString &name);
    
    // 模型数据
    void setConfigData(const ConfigParser::ConfigData &data);
    Q_INVOKABLE QModelIndex getIndex(const QString &id);
    ConfigParser::ConfigData getConfigData() const;
    
    // 获取所有菜单项（扁平化，用于显示）
    Q_INVOKABLE QVariantList getAllItems() const;
    
signals:
    void errorOccurred(const QString &message);
    
private:
    struct TreeItem {
        QString id;
        QString name;
        QString nameLocal;
        QString comment;
        QString commentLocal;
        QStringList menuTypes;
        QStringList supportSuffix;
        int positionNumber;
        QString execCommand;
        QStringList childActions;  // 子节点ID列表
        bool isRoot;
        int level;
        QString configFile;
        bool isSystem;
        int row;  // 在父节点中的位置
        
        // 根节点属性
        QString version;
        
        // 用于构建树形结构的指针
        TreeItem *parentItem;
        QList<TreeItem*> subItems;
    };
    
    TreeItem *m_rootItem;
    QList<TreeItem*> m_allItems;  // 所有节点，用于内存管理
    
    TreeItem *getItem(const QModelIndex &idx) const;
    QString generateUniqueId();
    
    // 从 ConfigData 构建 TreeItem 树
    void buildTree(const ConfigParser::ConfigData &data);
    void clearTree();
};

#endif // MENUTREEMODEL_H
