#include "menu_tree_model.h"
#include <QDateTime>

MenuTreeModel::MenuTreeModel(QObject *parent)
    : QAbstractItemModel(parent) {
    m_rootItem = new TreeItem();
    m_rootItem->isRoot = true;
    m_rootItem->level = 0;
    m_rootItem->id = "root";
    m_rootItem->parentItem = nullptr;
    m_allItems.append(m_rootItem);
}

MenuTreeModel::~MenuTreeModel() {
    clearTree();
}

void MenuTreeModel::clearTree() {
    qDeleteAll(m_allItems);
    m_allItems.clear();
    m_rootItem = nullptr;
}

MenuTreeModel::TreeItem* MenuTreeModel::getItem(const QModelIndex &idx) const {
    if (idx.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(idx.internalPointer());
        if (item) {
            return item;
        }
    }
    return m_rootItem;
}

QModelIndex MenuTreeModel::index(int row, int column, 
                                 const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    
    TreeItem *parentItem = getItem(parent);
    if (row >= parentItem->subItems.size()) {
        return QModelIndex();
    }
    
    TreeItem *childItem = parentItem->subItems.at(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    
    return QModelIndex();
}

QModelIndex MenuTreeModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) {
        return QModelIndex();
    }
    
    TreeItem *childItem = getItem(child);
    TreeItem *parentItem = childItem->parentItem;
    
    if (!parentItem || parentItem == m_rootItem) {
        return QModelIndex();
    }
    
    return createIndex(parentItem->row, 0, parentItem);
}

int MenuTreeModel::rowCount(const QModelIndex &parent) const {
    TreeItem *parentItem = getItem(parent);
    return parentItem->subItems.size();
}

int MenuTreeModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 1;
}

QVariant MenuTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    
    TreeItem *item = getItem(index);
    if (!item) {
        return QVariant();
    }
    
    switch (role) {
    case NameRole:
        return item->name;
    case NameLocalRole:
        return item->nameLocal.isEmpty() ? item->name : item->nameLocal;
    case IdRole:
        return item->id;
    case LevelRole:
        return item->level;
    case HasChildrenRole:
        return !item->subItems.isEmpty();
    case IsEditableRole:
        return !item->isSystem;
    case IsSystemRole:
        return item->isSystem;
    case ExecCommandRole:
        return item->execCommand;
    case MenuTypesRole:
        return item->menuTypes;
    case SupportSuffixRole:
        return item->supportSuffix;
    case PositionNumberRole:
        return item->positionNumber;
    case CommentRole:
        return item->comment;
    case CommentLocalRole:
        return item->commentLocal;
    case VersionRole:
        return item->version;
    case Qt::DisplayRole:
        return item->nameLocal.isEmpty() ? item->name : item->nameLocal;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MenuTreeModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[NameLocalRole] = "nameLocal";
    roles[IdRole] = "id";
    roles[LevelRole] = "level";
    roles[HasChildrenRole] = "hasChildren";
    roles[IsEditableRole] = "isEditable";
    roles[IsSystemRole] = "isSystem";
    roles[ExecCommandRole] = "execCommand";
    roles[MenuTypesRole] = "menuTypes";
    roles[SupportSuffixRole] = "supportSuffix";
    roles[PositionNumberRole] = "positionNumber";
    roles[CommentRole] = "comment";
    roles[CommentLocalRole] = "commentLocal";
    roles[VersionRole] = "version";
    return roles;
}

void MenuTreeModel::addItem(const QModelIndex &parent, const QString &name) {
    if (!parent.isValid()) {
        emit errorOccurred("无效的父项");
        return;
    }
    
    TreeItem *parentItem = getItem(parent);
    
    // 检查层级限制
    if (parentItem->level >= 3) {
        emit errorOccurred("最多支持3级菜单");
        return;
    }
    
    beginInsertRows(parent, parentItem->subItems.size(), 
                   parentItem->subItems.size());
    
    // 创建新菜单项
    TreeItem *newItem = new TreeItem();
    newItem->id = generateUniqueId();
    newItem->name = name;
    newItem->nameLocal = name;
    newItem->level = parentItem->level + 1;
    newItem->positionNumber = parentItem->subItems.size() + 1;
    newItem->configFile = parentItem->configFile;
    newItem->isSystem = parentItem->isSystem;
    newItem->parentItem = parentItem;
    newItem->row = parentItem->subItems.size();
    
    parentItem->subItems.append(newItem);
    m_allItems.append(newItem);
    
    endInsertRows();
}

void MenuTreeModel::removeItem(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    
    TreeItem *item = getItem(index);
    if (item->isSystem) {
        emit errorOccurred("系统配置不能删除");
        return;
    }
    
    QModelIndex parent = index.parent();
    TreeItem *parentItem = getItem(parent);
    
    if (!parentItem) {
        return;
    }
    
    int row = index.row();
    
    beginRemoveRows(parent, row, row);
    
    parentItem->subItems.removeAt(row);
    m_allItems.removeAll(item);
    delete item;
    
    // 更新后续节点的row
    for (int i = row; i < parentItem->subItems.size(); ++i) {
        parentItem->subItems[i]->row = i;
    }
    
    endRemoveRows();
}

void MenuTreeModel::moveItem(const QModelIndex &index, int direction) {
    if (!index.isValid()) {
        return;
    }
    
    QModelIndex parent = index.parent();
    TreeItem *parentItem = getItem(parent);
    
    if (!parentItem) {
        return;
    }
    
    int row = index.row();
    int newRow = row + direction;
    
    if (newRow < 0 || newRow >= parentItem->subItems.size()) {
        return;
    }
    
    // 交换位置
    beginMoveRows(parent, row, row, parent, direction > 0 ? newRow + 1 : newRow);
    
    parentItem->subItems.swapItemsAt(row, newRow);
    
    // 更新row
    parentItem->subItems[row]->row = row;
    parentItem->subItems[newRow]->row = newRow;
    
    endMoveRows();
}

void MenuTreeModel::updateItem(const QModelIndex &index, const QString &role, 
                               const QVariant &value) {
    if (!index.isValid()) {
        return;
    }
    
    TreeItem *item = getItem(index);
    if (!item) {
        return;
    }
    
    if (role == "name") {
        item->name = value.toString();
    } else if (role == "nameLocal") {
        item->nameLocal = value.toString();
    } else if (role == "execCommand") {
        item->execCommand = value.toString();
    } else if (role == "menuTypes") {
        item->menuTypes = value.toStringList();
    } else if (role == "supportSuffix") {
        item->supportSuffix = value.toStringList();
    } else if (role == "positionNumber") {
        item->positionNumber = value.toInt();
    }
    
    emit dataChanged(index, index);
}

void MenuTreeModel::buildTree(const ConfigParser::ConfigData &data) {
    // 使用 BFS 从根节点开始构建树
    QSet<QString> visited;
    QList<QPair<TreeItem*, QStringList>> queue;  // (父节点, 子节点ID列表)
    
    // 创建根节点
    if (data.actionMap.contains("root")) {
        const MenuActionItem *rootAction = data.actionMap["root"];
        m_rootItem->id = rootAction->id;
        m_rootItem->name = rootAction->name;
        m_rootItem->nameLocal = rootAction->nameLocal;
        m_rootItem->comment = rootAction->comment;
        m_rootItem->commentLocal = rootAction->commentLocal;
        m_rootItem->level = 0;
        m_rootItem->isRoot = true;
        m_rootItem->configFile = rootAction->configFile;
        m_rootItem->isSystem = rootAction->isSystem;
        m_rootItem->childActions = rootAction->childActions;
        m_rootItem->version = data.version;  // 添加版本号
        
        queue.append(qMakePair(m_rootItem, rootAction->childActions));
        visited.insert("root");
    }
    
    while (!queue.isEmpty()) {
        auto pair = queue.takeFirst();
        TreeItem *parentItem = pair.first;
        const QStringList &childIds = pair.second;
        
        for (int i = 0; i < childIds.size(); ++i) {
            const QString &childId = childIds[i];
            if (data.actionMap.contains(childId) && !visited.contains(childId)) {
                const MenuActionItem *childAction = data.actionMap[childId];
                
                // 创建新节点
                TreeItem *childItem = new TreeItem();
                childItem->id = childAction->id;
                childItem->name = childAction->name;
                childItem->nameLocal = childAction->nameLocal;
                childItem->comment = childAction->comment;
                childItem->commentLocal = childAction->commentLocal;
                childItem->level = parentItem->level + 1;
                childItem->isRoot = childAction->isRoot;
                childItem->configFile = childAction->configFile;
                childItem->isSystem = childAction->isSystem;
                childItem->positionNumber = childAction->positionNumber;
                childItem->execCommand = childAction->execCommand;
                childItem->menuTypes = childAction->menuTypes;
                childItem->supportSuffix = childAction->supportSuffix;
                childItem->childActions = childAction->childActions;
                childItem->parentItem = parentItem;
                childItem->row = i;
                
                parentItem->subItems.append(childItem);
                m_allItems.append(childItem);
                
                // 将子节点的子项添加到队列
                if (!childAction->childActions.isEmpty()) {
                    queue.append(qMakePair(childItem, childAction->childActions));
                }
                
                visited.insert(childId);
            }
        }
    }
    
    qDebug() << "buildTree: Built tree with" << m_allItems.size() << "items";
}

void MenuTreeModel::setConfigData(const ConfigParser::ConfigData &data) {
    beginResetModel();
    
    // 清空旧数据
    clearTree();
    
    // 创建新的根节点
    m_rootItem = new TreeItem();
    m_rootItem->isRoot = true;
    m_rootItem->level = 0;
    m_rootItem->id = "root";
    m_rootItem->parentItem = nullptr;
    m_allItems.append(m_rootItem);
    
    // 构建树
    buildTree(data);
    
    endResetModel();
}

QModelIndex MenuTreeModel::getIndex(const QString &id) {
    // 使用 BFS 查找节点
    QList<TreeItem*> queue;
    queue.append(m_rootItem);
    
    while (!queue.isEmpty()) {
        TreeItem *item = queue.takeFirst();
        if (item->id == id) {
            if (item->parentItem) {
                return createIndex(item->row, 0, item);
            }
            return QModelIndex();
        }
        
        for (TreeItem *child : item->subItems) {
            queue.append(child);
        }
    }
    
    return QModelIndex();
}

QVariantList MenuTreeModel::getAllItems() const {
    QVariantList result;
    QList<TreeItem*> queue;
    
    if (m_rootItem) {
        queue.append(m_rootItem);
    }
    
    while (!queue.isEmpty()) {
        TreeItem* item = queue.takeFirst();
        
        QVariantMap itemMap;
        itemMap["id"] = item->id;
        itemMap["name"] = item->name;
        itemMap["nameLocal"] = item->nameLocal.isEmpty() ? item->name : item->nameLocal;
        itemMap["level"] = item->level;
        itemMap["hasChildren"] = !item->subItems.isEmpty();
        itemMap["isEditable"] = !item->isSystem;
        itemMap["isSystem"] = item->isSystem;
        itemMap["execCommand"] = item->execCommand;
        itemMap["menuTypes"] = item->menuTypes;
        itemMap["supportSuffix"] = item->supportSuffix;
        itemMap["positionNumber"] = item->positionNumber;
        
        result.append(itemMap);
        
        for (TreeItem *child : item->subItems) {
            queue.append(child);
        }
    }
    
    qDebug() << "MenuTreeModel::getAllItems() returned" << result.size() << "items";
    return result;
}

QString MenuTreeModel::generateUniqueId() {
    static int counter = 0;
    return QString("action_%1_%2").arg(QDateTime::currentMSecsSinceEpoch())
                                     .arg(counter++);
}
