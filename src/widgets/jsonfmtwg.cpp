// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "jsonfmtwg.h"
#include "common/common.h"
#include "ui_jsonfmtwg.h"

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
    m_copyMenu->addAction(tr("All Data"), this, &JsonFormatWG::copyAllData);

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

void JsonFormatWG::copyAllData()
{
    QString allData = getAllData();
    if (!allData.isEmpty()) {
        QApplication::clipboard()->setText(allData);
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
    ui->stackedWidget->setCurrentIndex((ui->stackedWidget->currentIndex() + 1) % ui->stackedWidget->count());
}

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
