// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "jsonfmtwg.h"
#include "common/common.h"
#include "common/zjsonconfig.h"
#include "ui_jsonfmtwg.h"

#include <QMessageBox>
#include <QCloseEvent>

JsonFormatWG::JsonFormatWG(QWidget *parent)
    : BaseFormatWG(parent)
    , ui(new Ui::JsonFormatWG)
{
    initialize();
}

void JsonFormatWG::initUI()
{
    // Setup UI
    ui->setupUi(this);

    // Initialize models
    m_model = new QJsonModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setRecursiveFilteringEnabled(true);
    m_proxyModel->setFilterRole(Qt::DisplayRole);

    // Configure tree view    
    ui->treeView->setModel(m_proxyModel);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Initialize search component
    m_searchWG = new SearchWG(this);
    m_searchWG->setWindowTitle(tr("JSON Search"));
    
    // Configure search component display group boxes
    auto requiredBoxes = SearchWG::MatchControl | SearchWG::Operation;
    m_searchWG->setVisibleGroupBoxes(requiredBoxes);
    
    // Add search component to layout
    ui->treeViewLayout->addWidget(m_searchWG);
    m_searchWG->setVisible(false);

    // Set text view to read-only by default (enabled in edit mode)
    ui->textView->setReadOnly(true);

    // Default insert key-value
    m_defaultInsertKey = tr("new_key");
    m_defaultInsertValue = tr("new_value");
}

void JsonFormatWG::initMenu()
{
    // Add menu for text view
    ui->textView->addContextSeparator();

    m_searchAction = new QAction(tr("Search"), m_contextMenu);
    m_switchViewAction = new QAction(tr("Switch View"), m_contextMenu);

    ui->textView->addContextAction(m_switchViewAction);

    // Create copy submenu
    m_copyMenu = new QMenu(tr("Copy"), m_contextMenu);
    m_copyMenu->addAction(tr("Value"), this, &JsonFormatWG::copyValue);
    m_copyMenu->addAction(tr("Key"), this, &JsonFormatWG::copyKey);
    m_copyMenu->addAction(tr("Key-Value"), this, &JsonFormatWG::copyKeyValue);
    m_copyMenu->addSeparator();
    m_copyMenu->addAction(tr("All Data"), this, &BaseFormatWG::copyAllData);
    m_copyMenu->addSeparator();
    m_copyMenu->addAction(tr("Cmd"), this, &BaseFormatWG::copyCmd);
    m_contextMenu->addMenu(m_copyMenu);

    // Tree view operation menu
    m_contextMenu->addSeparator();
    m_expandMenu = new QMenu(tr("Expand"), m_contextMenu);
    m_expandMenu->addAction(tr("Expand"), this, &JsonFormatWG::expand);
    m_expandMenu->addAction(tr("Expand All"), this, &JsonFormatWG::expandAll);
    m_contextMenu->addMenu(m_expandMenu);

    m_collapseMenu = new QMenu(tr("Collapse"), m_contextMenu);
    m_collapseMenu->addAction(tr("Collapse"), this, &JsonFormatWG::collapse);
    m_collapseMenu->addAction(tr("Collapse All"), this, &JsonFormatWG::collapseAll);
    m_contextMenu->addMenu(m_collapseMenu);

    // Edit mode menu
    m_contextMenu->addSeparator();

    // Enable/Disable edit mode action
    m_editModeAction = new QAction(tr("Enable Edit"), m_contextMenu);
    m_contextMenu->addAction(m_editModeAction);

    // Insert menu (hidden by default, shown only in edit mode)
    m_insertMenu = new QMenu(tr("Insert"), m_contextMenu);
    m_insertAction = m_insertMenu->addAction(tr("Insert Item"), this, &JsonFormatWG::insertItem);
    m_insertChildAction = m_insertMenu->addAction(tr("Insert Child Item"), this, &JsonFormatWG::insertChildItem);

    // Delete action (hidden by default, shown only in edit mode)
    m_deleteAction = new QAction(tr("Delete"), m_contextMenu);
    connect(m_deleteAction, &QAction::triggered, this, &JsonFormatWG::deleteItem);

    // Initially hide edit mode actions
    m_insertMenu->menuAction()->setVisible(false);
    m_deleteAction->setVisible(false);

    // Basic operation menu
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_searchAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_switchViewAction);
}

void JsonFormatWG::initConnection()
{
    // Set right-click menu policy for tree view
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &JsonFormatWG::showContextMenu);

    connect(m_searchShortcut, &QShortcut::activated, this, [this]() {
        m_searchWG->setVisible(!m_searchWG->isVisible());
    });

    // Connect search related signals
    connect(m_searchWG, &SearchWG::searchReady, this, &JsonFormatWG::on_searchReady);
    connect(m_searchWG, &SearchWG::matchControlChanged, this, &JsonFormatWG::on_searchReady);
    connect(m_searchWG, &SearchWG::searchTextChanged, this, &JsonFormatWG::on_searchTextChanged);
    connect(m_searchWG, &SearchWG::searchClear, this, &JsonFormatWG::on_searchClear);

    connect(m_switchViewAction, &QAction::triggered, this, &JsonFormatWG::toggleSwitchView);
    connect(m_searchAction, &QAction::triggered, this, &JsonFormatWG::toggleSearch);

    // Edit mode connections
    connect(m_editModeAction, &QAction::triggered, this, &JsonFormatWG::toggleEditMode);

    // Model data change connection for auto-save
    connect(m_model, &QJsonModel::dataChanged, this, &JsonFormatWG::onModelDataChanged);
}

void JsonFormatWG::initShortCut()
{
    // Set search shortcut
    m_searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
}

void JsonFormatWG::initExtra()
{
    // Additional initialization code (if any)
    // Currently JsonFormatWG has no additional initialization requirements
}

JsonFormatWG::~JsonFormatWG()
{
    delete ui;
}

void JsonFormatWG::setControlMargin(const int &top, const int &bottom, const int &left, const int &right)
{
    ui->verticalLayout->setContentsMargins(left, top, right, bottom);
}

bool JsonFormatWG::loadJson(const QByteArray &json)
{
    bool res = m_model->loadJson(json);
    ui->treeView->expandAll();
    ui->textView->setPlainText(json);
    return res;
}

void JsonFormatWG::showContextMenu(const QPoint &pos)
{
    // Get the index at the clicked position
    QModelIndex index = ui->treeView->indexAt(pos);
    
    // Check if there is a valid selection
    if (index.isValid() || !ui->treeView->selectionModel()->selectedIndexes().isEmpty()) {
        BaseFormatWG::showContextMenu(pos);
    }
}

void JsonFormatWG::copyValue()
{
    QModelIndexList selectedIndexes = getSelectedIndexes();
    if (selectedIndexes.isEmpty()) return;

    QStringList values;
    for (const QModelIndex &proxyIndex : selectedIndexes) {
        if (proxyIndex.column() == 0) {
            QString value = getValueForIndex(proxyIndex);
            if (!value.isEmpty()) {
                values.append(value);
            }
        }
    }

    if (!values.isEmpty()) {
        QApplication::clipboard()->setText(values.join("\n"));
    }
}

void JsonFormatWG::copyKeyValue()
{
    QModelIndexList selectedIndexes = getSelectedIndexes();
    if (selectedIndexes.isEmpty()) return;

    QStringList keyValues;
    for (const QModelIndex &proxyIndex : selectedIndexes) {
        if (proxyIndex.column() == 0) {
            QString key = getKeyForIndex(proxyIndex);
            QString value = getValueForIndex(proxyIndex);
            if (!key.isEmpty() && !value.isEmpty()) {
                keyValues.append(QString("%1: %2").arg(key).arg(value));
            } else if (!key.isEmpty()) {
                keyValues.append(key);
            } else if (!value.isEmpty()) {
                keyValues.append(value);
            }
        }
    }

    if (!keyValues.isEmpty()) {
        QApplication::clipboard()->setText(keyValues.join("\n"));
    }
}

void JsonFormatWG::copyKey()
{
    QModelIndexList selectedIndexes = getSelectedIndexes();
    if (selectedIndexes.isEmpty()) return;

    QStringList keys;
    for (const QModelIndex &proxyIndex : selectedIndexes) {
        if (proxyIndex.column() == 0) {
            QString key = getKeyForIndex(proxyIndex);
            if (!key.isEmpty()) {
                keys.append(key);
            }
        }
    }

    if (!keys.isEmpty()) {
        QApplication::clipboard()->setText(keys.join("\n"));
    }
}

void JsonFormatWG::expand()
{
    if (ui->treeView->currentIndex().isValid()) {
        ui->treeView->expand(ui->treeView->currentIndex());
    }
}

void JsonFormatWG::expandAll()
{
    ui->treeView->expandAll();
}

void JsonFormatWG::collapse()
{
    if (ui->treeView->currentIndex().isValid()) {
        ui->treeView->collapse(ui->treeView->currentIndex());
    }
}

void JsonFormatWG::collapseAll()
{
    ui->treeView->collapseAll();
}

void JsonFormatWG::toggleSearch()
{
    if (m_searchWG) {
        m_searchWG->setVisible(!m_searchWG->isVisible());
        if (m_searchWG->isVisible()) {
            m_searchWG->setFocus();
        }
    }
}

void JsonFormatWG::toggleSwitchView()
{
    int currentIndex = ui->stackedWidget->currentIndex();
    int nextIndex = (currentIndex + 1) % ui->stackedWidget->count();

    // When switching from text view (index 1) to tree view (index 0),
    // parse the text and reload the tree model
    if (currentIndex == 1) {
        QString text = ui->textView->toPlainText();
        QByteArray jsonBytes = text.toUtf8();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            // Reload the tree model with the edited JSON
            m_model->loadJson(jsonBytes);
            ui->treeView->expandAll();

            // Save to config only in edit mode
            if (m_editMode) {
                saveConfig();
                emit dataChanged();
            }
        } else if (m_editMode) {
            QMessageBox::warning(this, tr("JSON Parse Error"),
                                   tr("Failed to parse JSON: %1\n\nSwitching to tree view anyway, but text changes will be lost.")
                                   .arg(parseError.errorString()));
        }
    }

    // When switching from tree view (index 0) to text view (index 1),
    // update the text view with the current model data
    if (currentIndex == 0) {
        QByteArray jsonData = m_model->json(false);
        ui->textView->setPlainText(QString::fromUtf8(jsonData));
    }

    ui->stackedWidget->setCurrentIndex(nextIndex);
}

// ==================== Edit Mode Methods ====================

void JsonFormatWG::setEditMode(bool enabled)
{
    if (m_editMode == enabled) return;
    m_editMode = enabled;
    m_model->setEditable(enabled);
    ui->textView->setReadOnly(!enabled);
    updateEditModeUI();
    emit editModeChanged(enabled);
}

bool JsonFormatWG::isEditMode() const
{
    return m_editMode;
}

void JsonFormatWG::setConfigSavePath(const QString &group)
{
    m_configSavePath = group;
}

QString JsonFormatWG::configSavePath() const
{
    return m_configSavePath;
}

void JsonFormatWG::toggleEditMode()
{
    if (m_editMode) {
        // Check for default values before disabling edit mode
        if (checkDefaultValues()) {
            setEditMode(false);
        }
    } else {
        setEditMode(true);
    }
}

void JsonFormatWG::updateEditModeUI()
{
    if (m_editMode) {
        m_editModeAction->setText(tr("Disable Edit"));
        m_insertMenu->menuAction()->setVisible(true);
        m_deleteAction->setVisible(true);

        // Add the insert menu and delete action to context menu if not already there
        if (!m_contextMenu->actions().contains(m_insertMenu->menuAction())) {
            // Insert after the edit mode action
            QList<QAction*> actions = m_contextMenu->actions();
            int idx = actions.indexOf(m_editModeAction);
            if (idx >= 0) {
                m_contextMenu->insertAction(actions.value(idx + 1), m_insertMenu->menuAction());
                m_contextMenu->insertAction(m_insertMenu->menuAction(), m_deleteAction);
            } else {
                m_contextMenu->addMenu(m_insertMenu);
                m_contextMenu->addAction(m_deleteAction);
            }
        }
    } else {
        m_editModeAction->setText(tr("Enable Edit"));
        m_insertMenu->menuAction()->setVisible(false);
        m_deleteAction->setVisible(false);

        // Remove from context menu
        m_contextMenu->removeAction(m_insertMenu->menuAction());
        m_contextMenu->removeAction(m_deleteAction);
    }
}

void JsonFormatWG::insertItem()
{
    if (!m_editMode) return;

    // Get the current selection to determine parent
    QModelIndex currentIndex = ui->treeView->currentIndex();
    QModelIndex sourceIndex = m_proxyModel->mapToSource(currentIndex);

    // Determine parent: if the selected item is an object/array, insert as child;
    // otherwise insert as sibling
    QModelIndex parentIndex;
    QJsonTreeItem *item = getItemForIndex(currentIndex);

    if (item && (item->type() == QJsonValue::Object || item->type() == QJsonValue::Array)) {
        parentIndex = sourceIndex;
    } else if (sourceIndex.isValid()) {
        parentIndex = sourceIndex.parent();
    }

    // Directly insert with default values, user can edit in-place in the tree
    if (m_model->insertItem(parentIndex, m_defaultInsertKey, m_defaultInsertValue)) {
        ui->treeView->expandAll();

        // Find the newly inserted item and start editing it
        QJsonTreeItem *parentItem = nullptr;
        if (!parentIndex.isValid())
            parentItem = m_model->rootItem();
        else
            parentItem = static_cast<QJsonTreeItem*>(parentIndex.internalPointer());

        if (parentItem) {
            int newRow = parentItem->childCount() - 1;
            QModelIndex newSourceIndex = m_model->index(newRow, 0, parentIndex);
            QModelIndex newProxyIndex = m_proxyModel->mapFromSource(newSourceIndex);
            if (newProxyIndex.isValid()) {
                ui->treeView->setCurrentIndex(newProxyIndex);
                ui->treeView->edit(newProxyIndex);
            }
        }

        saveConfig();
        emit dataChanged();
    }
}

void JsonFormatWG::insertChildItem()
{
    if (!m_editMode) return;

    QModelIndex currentIndex = ui->treeView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, tr("Insert Child Item"), tr("Please select a parent item first."));
        return;
    }

    QModelIndex sourceIndex = m_proxyModel->mapToSource(currentIndex);
    QJsonTreeItem *item = getItemForIndex(currentIndex);

    if (!item || (item->type() != QJsonValue::Object && item->type() != QJsonValue::Array)) {
        QMessageBox::warning(this, tr("Insert Child Item"), tr("Cannot insert child into a non-object/non-array item."));
        return;
    }

    // Directly insert with default values, user can edit in-place in the tree
    if (m_model->insertItem(sourceIndex, m_defaultInsertKey, m_defaultInsertValue)) {
        ui->treeView->expandAll();

        // Find the newly inserted item and start editing it
        QJsonTreeItem *parentItem = static_cast<QJsonTreeItem*>(sourceIndex.internalPointer());
        if (parentItem) {
            int newRow = parentItem->childCount() - 1;
            QModelIndex newSourceIndex = m_model->index(newRow, 0, sourceIndex);
            QModelIndex newProxyIndex = m_proxyModel->mapFromSource(newSourceIndex);
            if (newProxyIndex.isValid()) {
                ui->treeView->setCurrentIndex(newProxyIndex);
                ui->treeView->edit(newProxyIndex);
            }
        }

        saveConfig();
        emit dataChanged();
    }
}

void JsonFormatWG::deleteItem()
{
    if (!m_editMode) return;

    QModelIndexList selectedIndexes = ui->treeView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, tr("Delete"), tr("Please select an item to delete."));
        return;
    }

    // Confirm deletion
    int ret = QMessageBox::question(this, tr("Delete"),
                                      tr("Are you sure you want to delete the selected item(s)?"),
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    // Collect source indexes to delete (unique, column 0 only)
    QList<QModelIndex> sourceIndexes;
    for (const QModelIndex &proxyIdx : selectedIndexes) {
        if (proxyIdx.column() == 0) {
            QModelIndex sourceIdx = m_proxyModel->mapToSource(proxyIdx);
            if (sourceIdx.isValid() && !sourceIndexes.contains(sourceIdx)) {
                sourceIndexes.append(sourceIdx);
            }
        }
    }

    // Delete from bottom to top to avoid index invalidation
    for (int i = sourceIndexes.size() - 1; i >= 0; --i) {
        m_model->removeItem(sourceIndexes[i]);
    }

    saveConfig();
    emit dataChanged();
}

void JsonFormatWG::onModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);
    Q_UNUSED(roles);

    if (m_editMode) {
        saveConfig();
        emit dataChanged();
    }
}

void JsonFormatWG::saveConfig()
{
    if (m_configSavePath.isEmpty()) return;

    QByteArray jsonData = m_model->json(false);
    if (jsonData.isEmpty()) return;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) return;

    QJsonObject groupData = doc.object();
    ZJsonConfig::instance()->setGroup(m_configSavePath, groupData);
}

bool JsonFormatWG::checkDefaultValues()
{
    // Check if any item in the model still has the default key-value pair
    QJsonTreeItem *root = m_model->rootItem();
    if (!root) return true;

    bool foundDefault = false;
    for (int i = 0; i < root->childCount(); ++i) {
        QJsonTreeItem *child = root->child(i);
        if (child && child->key() == m_defaultInsertKey && 
            child->value().toString() == m_defaultInsertValue) {
            foundDefault = true;
            break;
        }
        // Also check children recursively
        for (int j = 0; j < child->childCount(); ++j) {
            QJsonTreeItem *grandChild = child->child(j);
            if (grandChild && grandChild->key() == m_defaultInsertKey && 
                grandChild->value().toString() == m_defaultInsertValue) {
                foundDefault = true;
                break;
            }
        }
        if (foundDefault) break;
    }

    if (foundDefault) {
        int ret = QMessageBox::warning(this, tr("Unmodified Default Value"),
                                         tr("The default key-value pair (\"%1\": \"%2\") has not been modified. "
                                            "Do you want to save anyway?")
                                         .arg(m_defaultInsertKey).arg(m_defaultInsertValue),
                                         QMessageBox::Save | QMessageBox::Cancel, QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            saveConfig();
            return true;
        }
        return false;
    }

    saveConfig();
    return true;
}

void JsonFormatWG::closeEvent(QCloseEvent *event)
{
    if (m_editMode && !checkDefaultValues()) {
        event->ignore();
        return;
    }

    if (m_editMode) {
        saveConfig();
    }

    event->accept();
}

// ==================== End Edit Mode Methods ====================

QJsonTreeItem* JsonFormatWG::getItemForIndex(const QModelIndex &proxyIndex)
{
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    if (sourceIndex.isValid()) {
        return static_cast<QJsonTreeItem*>(sourceIndex.internalPointer());
    }
    return nullptr;
}

QString JsonFormatWG::getKeyForIndex(const QModelIndex &proxyIndex)
{
    QJsonTreeItem* item = getItemForIndex(proxyIndex);
    if (item) {
        return item->key();
    }
    return QString();
}

QString JsonFormatWG::getValueForIndex(const QModelIndex &proxyIndex)
{
    QJsonTreeItem* item = getItemForIndex(proxyIndex);
    if (item) {
        QVariant value = item->value();
        if (value.isValid()) {
            switch (item->type()) {
            case QJsonValue::Bool:
                return value.toBool() ? "true" : "false";
            case QJsonValue::Double:
                return QString::number(value.toDouble());
            case QJsonValue::String:
                return value.toString();
            case QJsonValue::Array:
                return "[Array]";
            case QJsonValue::Object:
                return "[Object]";
            case QJsonValue::Null:
                return "null";
            default:
                return value.toString();
            }
        }
    }
    return QString();
}

// Implementation of base class virtual functions
QModelIndexList JsonFormatWG::getSelectedIndexes()
{
    return ui->treeView->selectionModel()->selectedIndexes();
}

bool JsonFormatWG::hasValidSelection()
{
    return !ui->treeView->selectionModel()->selectedIndexes().isEmpty();
}

QString JsonFormatWG::getAllData()
{
    QByteArray jsonData = m_model->json(false); // false means no compression, formatted output
    if (!jsonData.isEmpty()) {
        return QString::fromUtf8(jsonData);
    }
    return QString();
}

void JsonFormatWG::on_searchReady()
{
    QString searchText = m_searchWG->getSearchText().trimmed();
    if (searchText.isEmpty()) {
        m_searchWG->setSearchStatus(tr("Search text is empty"));
        return;
    }

    // Check if there is content to search
    if (m_model->rowCount() == 0) {
        m_searchWG->setSearchStatus(tr("No content to search"));
        return;
    }

    if (m_searchWG->isCaseSensitive()) {
        m_proxyModel->setFilterCaseSensitivity(Qt::CaseSensitive);
    } else {
        m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    }

    m_proxyModel->setFilterKeyColumn(-1); // Search all columns
    m_proxyModel->setFilterRole(Qt::DisplayRole);

    if (m_searchWG->isUseRegularExpression()) {
        if (m_searchWG->isMatchWholewords()) {
            // For whole word matching with regex, we need to adjust the pattern
            searchText = QString("\\b%1\\b").arg(searchText);
        }
        QT_SET_FILTER_REGEXP(m_proxyModel, searchText);
    } else {
        if (m_searchWG->isMatchWholewords()) {
            // For whole word matching without regex, we need a different approach
            // Since QSortFilterProxyModel doesn't support whole word matching directly,
            // we'll use a regex pattern for this case too
            searchText = QString("\\b%1\\b").arg(QRegularExpression::escape(searchText));
            QT_SET_FILTER_REGEXP(m_proxyModel, searchText);
        } else {
            m_proxyModel->setFilterFixedString(searchText);
        }
    }

    // Update search status
    int visibleCount = 0;
    int totalCount = 0;
    countVisibleAndTotalItems(m_proxyModel, QModelIndex(), visibleCount, totalCount);

    if (visibleCount > 0) {
        m_searchWG->setSearchStatus(tr("Found %1 of %2 items").arg(visibleCount).arg(totalCount));
        ui->treeView->expandAll();
    } else {
        m_searchWG->setSearchStatus(tr("No items found"));
    }
}

void JsonFormatWG::on_searchTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        m_proxyModel->setFilterFixedString("");
        m_searchWG->setSearchStatus("");
    }
}

void JsonFormatWG::on_searchClear()
{
    m_proxyModel->setFilterFixedString("");
    m_searchWG->setSearchText("");
    m_searchWG->setSearchStatus("");
    ui->treeView->expandAll();
}

void JsonFormatWG::countVisibleAndTotalItems(QAbstractItemModel *model, const QModelIndex &parent, int &visibleCount, int &totalCount)
{
    for (int i = 0; i < model->rowCount(parent); ++i) {
        QModelIndex index = model->index(i, 0, parent);
        totalCount++;

        // Check if the item is visible in the proxy model
        bool itemVisible = false;
        if (model == m_proxyModel) {
            // If we're already working with the proxy model
            itemVisible = true;
        } else {
            // If we're working with the source model, check if it has a mapping in the proxy
            QModelIndex proxyIndex = m_proxyModel->mapFromSource(index);
            itemVisible = proxyIndex.isValid();
        }

        // Check children
        int childVisibleCount = 0;
        int childTotalCount = 0;
        if (model->hasChildren(index)) {
            QAbstractItemModel *childModel = model;
            QModelIndex childParent = index;

            // If we're using the source model, we need to work with the proxy model for children
            if (model == m_model) {
                QModelIndex proxyIndex = m_proxyModel->mapFromSource(index);
                if (proxyIndex.isValid()) {
                    childModel = m_proxyModel;
                    childParent = proxyIndex;
                } else {
                    childModel = nullptr;
                }
            }

            if (childModel) {
                countVisibleAndTotalItems(childModel, childParent, childVisibleCount, childTotalCount);
            }
        }

        if (itemVisible) {
            visibleCount += 1 + childVisibleCount;
        }
        totalCount += childTotalCount;
    }
}
