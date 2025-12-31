// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "tablefmtwg.h"
#include "common/zffmpeg.h"
#include "common/zffplay.h"
#include "qdebug.h"
#include "ui_tablefmtwg.h"
#include <QtGlobal>
#include <QPoint>
#include <QApplication>
#include <QClipboard>
#include <QMetaObject>
#include <QItemSelectionRange>
#include "progressdlg.h"

TableFormatWG::TableFormatWG(QWidget *parent)
    : BaseFormatWG(parent)
    , ui(new Ui::TableFormatWG)
{
    initialize();
}

void TableFormatWG::initUI()
{
    // Setup UI
    ui->setupUi(this);
    ui->detail_raw_pte->setVisible(false);

    m_isUserAdjusted = false;
    m_resizeTimer = new QTimer(this);
    m_lastTableWidth = 0;

    // Setup table model and view
    setupTableModel();
    
    // Setup column width management
    setupColumnWidthManagement();

    // Connect the action
    createDetailSearchDialog();

    // Set context menu policy for search button
    ui->search_btn->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->detail_tb->setContextMenuPolicy(Qt::CustomContextMenu);
}

void TableFormatWG::initMenu()
{
    // detail info action
    m_detailAction = new QAction("Detail Info", this);

    // restore order action
    m_restoreOrderAction = new QAction("Restore Order", this);
    m_contextMenu->addAction(m_restoreOrderAction);
    m_contextMenu->addSeparator();

    // fit column width action
    m_fitTableColumnAction = new QAction("Fit Column Width", this);
    m_contextMenu->addAction(m_fitTableColumnAction);

    // Add image menu to context menu
    m_imageMenu = new QMenu(tr("Image"), this);
    m_previewImageAction = new QAction(tr("Preview"), this);
    m_saveImageAction = new QAction(tr("Save"), this);
    m_imageMenu->addAction(m_previewImageAction);
    m_imageMenu->addAction(m_saveImageAction);
    m_contextMenu->addMenu(m_imageMenu);
    m_contextMenu->addSeparator();

    m_searchButtonMenu = new QMenu(this);
    m_detailSearchAction = new QAction(tr("Detail Search"), this);
    m_detailSearchAction->setToolTip(tr("Open advanced search dialog with more options"));
    m_searchButtonMenu->addAction(m_detailSearchAction);

    // copy selected text action
    m_copyMenu = new QMenu("Copy", this);
    m_copySelectedTextAction = new QAction("Select Text", this);
    m_copySelectedTextWithHeaderAction = new QAction("Select Text With Header", this);
     m_copyMenu->addAction(m_copySelectedTextAction);
     m_copyMenu->addAction(m_copySelectedTextWithHeaderAction);
    m_copyMenu->addSeparator();

    // copy selected rows action
    m_copySelectedRowsAction = new QAction("Selected Rows", this);
    m_copySelectedRowsWithHeaderAction = new QAction("Selected Rows With Header", this);
    m_copyMenu->addAction(m_copySelectedRowsAction);
    m_copyMenu->addAction(m_copySelectedRowsWithHeaderAction);
    m_copyMenu->addSeparator();

    // copy selected columns action
    m_copySelectedColumnsAction = new QAction("Selected Columns", this);
    m_copySelectedColumnsWithHeaderAction = new QAction("Selected Columns With Header", this);
    m_copyMenu->addAction(m_copySelectedColumnsAction);
    m_copyMenu->addAction(m_copySelectedColumnsWithHeaderAction);
    m_copyMenu->addSeparator();

    // copy all data action
    m_copyAllDataAction = new QAction("All Data", this);
    m_copyAllDataWithHeaderAction = new QAction("All Data With Header", this);
    m_copyMenu->addAction(m_copyAllDataAction);
    m_copyMenu->addAction(m_copyAllDataWithHeaderAction);

    // Add copy menu to context menu
    m_contextMenu->addMenu(m_copyMenu);
}

void TableFormatWG::initConnection()
{
    // Connect model data change signal
    connect(m_model, &QAbstractItemModel::dataChanged, this,
            [=](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
                emit dataChanged(m_data_tb.at(topLeft.row()));
            });

    connect(ui->detail_tb, &QTableView::customContextMenuRequested, [this](const QPoint &pos) {
        if (HELP_OPTION_FORMATS.contains(m_helpKey)) {
            m_contextMenu->addAction(m_detailAction);
        } else {
            m_contextMenu->removeAction(m_detailAction);
        }

        QModelIndex index = ui->detail_tb->indexAt(pos);

        if (index.isValid()) {
            m_currentRow = index.row();
            m_currentColumn = index.column();

            m_contextMenu->exec(ui->detail_tb->viewport()->mapToGlobal(pos));
        }
    });

    connect(m_contextMenu, &QMenu::aboutToShow, [=](){
        // Emit signal before showing context menu
        emit contextMenuAboutToShow();
    });

    connect(m_detailAction, &QAction::triggered, this, &TableFormatWG::showDetailInfo);

    connect(m_restoreOrderAction, &QAction::triggered, [=](){
        QSortFilterProxyModel *proxyModel = qobject_cast<QSortFilterProxyModel*>(ui->detail_tb->model());
        if (!proxyModel) return;
        proxyModel->sort(-1, Qt::AscendingOrder);
        // multiColumnSearchModel->sort(-1, Qt::AscendingOrder); // both two could satisify
        ui->detail_tb->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
        ui->detail_tb->horizontalHeader()->setSortIndicatorShown(false);
    });

    connect(m_fitTableColumnAction, &QAction::triggered, this, &TableFormatWG::fitTableColumnToContent);

    connect(m_detailSearchAction, &QAction::triggered, this, &TableFormatWG::showDetailSearch);

    connect(ui->search_btn, &QPushButton::customContextMenuRequested,
            this, [this](const QPoint &pos) {
                m_searchButtonMenu->exec(ui->search_btn->mapToGlobal(pos));
            });
    connect(m_copySelectedTextAction, &QAction::triggered, this, &TableFormatWG::copySelectedText);

    connect(m_copySelectedTextWithHeaderAction, &QAction::triggered, this, &TableFormatWG::copySelectedTextWithHeader);

    connect(m_copySelectedRowsAction, &QAction::triggered, this, &TableFormatWG::copySelectedRows);

    connect(m_copySelectedRowsWithHeaderAction, &QAction::triggered, this, &TableFormatWG::copySelectedRowsWithHeader);

    connect(m_copySelectedColumnsAction, &QAction::triggered, this, &TableFormatWG::copySelectedColumns);

    connect(m_copySelectedColumnsWithHeaderAction, &QAction::triggered, this, &TableFormatWG::copySelectedColumnsWithHeader);

    connect(m_copyAllDataAction, &QAction::triggered, this, &TableFormatWG::copyAllData);

    connect(m_copyAllDataWithHeaderAction, &QAction::triggered, this, &TableFormatWG::copyAllDataWithHeader);

    connect(m_previewImageAction, &QAction::triggered, this, &TableFormatWG::previewImage);

    connect(m_saveImageAction, &QAction::triggered, this, &TableFormatWG::saveImage);

    connect(this, &TableFormatWG::contextMenuAboutToShow, this, &TableFormatWG::onContextMenuAboutToShow);
}

void TableFormatWG::initExtra()
{
    // Additional initialization code (if any)
    // Currently TableFormatWG has no additional initialization requirements
}

void TableFormatWG::initShortCut()
{
    m_searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    // Setup search shortcut
    connect(m_searchShortcut, &QShortcut::activated, this, &TableFormatWG::showDetailSearch);
}

TableFormatWG::~TableFormatWG()
{
    delete m_headerManager;
    delete m_detailSearchDialog;
    delete ui;
}

void TableFormatWG::setHelpInfoKey(const QString &key)
{
    m_helpKey = key;
}

void TableFormatWG::setSearchTitleVisiable(const bool &visiable)
{
    ui->search_title_wg->setVisible(visiable);
}

QList<QStringList> TableFormatWG::getSelectLines()
{
    QList<QStringList> tmp;
    QSet<int> availableRow;
    QModelIndexList selectedRows = ui->detail_tb->selectionModel()->selectedIndexes();
    foreach (const QModelIndex &it, selectedRows) {
        auto index = multiColumnSearchModel->mapToSource(it);
        availableRow.insert(index.row());
    }

    foreach (auto &row, availableRow) {
        tmp.append(m_data_tb.at(row));
    }

    return tmp;
}

QList<int> TableFormatWG::getSelectRows()
{
    QList<int> tmp;
    QSet<int> availableRow;
    QModelIndexList selectedRows = ui->detail_tb->selectionModel()->selectedIndexes();
    foreach (const QModelIndex &it, selectedRows) {
        auto index = multiColumnSearchModel->mapToSource(it);
        availableRow.insert(index.row());
    }

    foreach (auto &row, availableRow) {
        tmp.append(row);
    }

    return tmp;
}

const QList<QStringList> *TableFormatWG::getTableData()
{
    return &m_data_tb;
}

void TableFormatWG::on_search_btn_clicked()
{
    onDetailSearchCompleted();
}

void TableFormatWG::clearDetailTb()
{
    m_model->setRow(0);
}

void TableFormatWG::initHeaderDetailTb(const QStringList &headers, QString format_join)
{
    m_headers = headers;
    
    m_model->setColumn(m_headers.count());
    m_model->setTableHeader(&m_headers);

    // Initial resize mode will be set in setupInitialColumnWidths()
    m_headerManager->restoreState();

    ui->detail_raw_pte->clear();
    ui->detail_raw_pte->appendPlainText(m_headers.join(format_join));
}

void TableFormatWG::updateDataDetailTb(const QList<QStringList> &data_tb, QString format_join)
{
    m_data_tb = data_tb;

    m_model->setRow(m_data_tb.count());
    m_model->setTableData(const_cast<QList<QStringList>*>(&m_data_tb));

    if (!ui->detail_tb->model()) {
        ui->detail_tb->setModel(multiColumnSearchModel);
    }
    
    ui->detail_tb->setShowGrid(true);

    updateCurrentModel();

    if (m_headers.size() > 0) {
        setupInitialColumnWidths();
    }
    ui->detail_raw_pte->clear();
    for (auto it : m_data_tb) {
        ui->detail_raw_pte->appendPlainText(it.join(format_join));
    }

    QTimer::singleShot(50, this, [this]() {
        resizeColumnsProportionally();
    });
}

void TableFormatWG::updateDataDetailTb(const QMap<QString, QList<QStringList> > &data_tb, QString format_join)
{
    m_data_tb.clear();
    for (auto key: data_tb.keys()) {
        m_data_tb.append(data_tb.value(key));
    }

    m_model->setRow(m_data_tb.count());
    m_model->setTableData(const_cast<QList<QStringList>*>(&m_data_tb));

    if (!ui->detail_tb->model()) {
        ui->detail_tb->setModel(multiColumnSearchModel);
    }

    ui->detail_tb->setShowGrid(true);

    updateCurrentModel();

    if (m_headers.size() > 0) {
        setupInitialColumnWidths();
    }
    ui->detail_raw_pte->clear();
    for (auto it : m_data_tb) {
        ui->detail_raw_pte->appendPlainText(it.join(format_join));
    }
}

void TableFormatWG::removeDataFromRowIndexs(const QList<int> &indexs)
{
    QList<int> sortedRows = indexs;
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
    foreach (int row, sortedRows) {
        m_data_tb.removeAt(row);
    }
}

void TableFormatWG::appendDataDetailTb(const QList<QStringList> &data_tb, QString format_join)
{
    m_data_tb.append(data_tb);

    m_model->setRow(m_data_tb.count());
    m_model->setTableData(const_cast<QList<QStringList>*>(&m_data_tb));

    if (!ui->detail_tb->model()) {
        ui->detail_tb->setModel(multiColumnSearchModel);
    }

    ui->detail_tb->setShowGrid(true);

    updateCurrentModel();

    if (m_headers.size() > 0) {
        setupInitialColumnWidths();
    }
    ui->detail_raw_pte->clear();
    for (auto it : m_data_tb) {
        ui->detail_raw_pte->appendPlainText(it.join(format_join));
    }
}

void TableFormatWG::removeSelectedRow()
{
    QSet<int> availableRow;
    QModelIndexList selectedRows = ui->detail_tb->selectionModel()->selectedIndexes();
    foreach (const QModelIndex &index, selectedRows) {
        availableRow.insert(index.row());
    }

    QList<int> rowsToDelete = availableRow.values();
    std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());

    foreach (int row, rowsToDelete) {
        if (row >= 0 && row < m_data_tb.size()) {
            m_data_tb.removeAt(row);
        }
    }

    m_model->setRow(m_data_tb.count());
}

void TableFormatWG::formatData(const QString &data, QList<QStringList> &data_tb, QStringList &headers, QString format_key)
{
    headers.clear();
    data_tb.clear();

    QStringList rawStringLines = data.split("\n", QT_SKIP_EMPTY_PARTS);

    if (rawStringLines.size() < 1)
        return;

    if (rawStringLines.at(0).contains(":"))
        rawStringLines.removeFirst();

    // -L -sources -sinks
    if (QStringList{"L", "sources", "sinks"}.contains(format_key, Qt::CaseInsensitive)){
        headers << tr("Info");
        for (auto it : rawStringLines) {
            data_tb << QStringList{it};
        }
        return;
    }

    // -version
    if (QStringList{"version"}.contains(format_key, Qt::CaseInsensitive)) {
        headers << tr("Config") << "Value";

        data_tb.append(QStringList{tr("Version"), rawStringLines.at(0)});
        data_tb.append(QStringList{tr("Build"), rawStringLines.at(1)});

        for (int i = 2; i < rawStringLines.size(); ++i) {
            QStringList tmp = rawStringLines.at(i).split(" ", QT_SKIP_EMPTY_PARTS);
            if (tmp.size() > 0 && tmp.at(0).contains("configuration"))
                tmp.takeFirst();

            QStringList tb_of_line;
            if (rawStringLines.at(i).contains("=")){
                for (auto it : tmp) {
                    QStringList keyParts = it.split("=", QT_SKIP_EMPTY_PARTS);
                    tb_of_line.clear();
                    if (keyParts.size() == 2) {
                        tb_of_line << keyParts[0].replace("--", "").trimmed() << keyParts.at(1).trimmed();
                    } else {
                        tb_of_line << "" << keyParts[0].replace("--", "").trimmed();
                    }
                    data_tb.append(tb_of_line);
                }
            } else {
                tb_of_line.append(tmp.at(0).trimmed());
                tmp.removeFirst();
                tb_of_line.append(tmp.join(" "));
                data_tb.append(tb_of_line);
            }
        }
        return;
    }

    // -formats -muxers -demuxers -devices -codecs -decoders -encoders -filters -pix_fmts
    if (QStringList{"formats", "muxers", "demuxers", "devices", "codecs", "decoders", "filters", "encoders", "pixfmts"}.contains(format_key, Qt::CaseInsensitive)) {
        QStringList parts{QString(""), QString("")};
        if (format_key.contains("filters")) {
            for (auto it : rawStringLines) {
                if (it.contains("=")) {
                    parts[0].append(it + "\n");
                } else {
                    parts[1].append(it + "\n");
                }
            }
        } else {
            parts = data.split("--", QT_SKIP_EMPTY_PARTS);
        }

        if (parts.size() >= 2) {

            // header
            QString head_code = "";
            auto tmp_headers = parts.at(0).split("\n", QT_SKIP_EMPTY_PARTS);
            for (auto it : tmp_headers) {
                if (it.contains(":"))
                    continue;

                if (it.contains("=")) {
                    QStringList tmp = it.split("=", QT_SKIP_EMPTY_PARTS);
                    if (tmp.size() >= 2) {
                        headers.append("[" +tmp[0].replace(".", "").trimmed() + "]" + tmp.at(1).trimmed());
                        head_code.append(tmp[0].replace(".", "").trimmed());
                    }
                }
            }
            headers.append(tr("name"));
            if (format_key.contains("pixfmts")) {
                headers.append(tr("NB_COMPONENTS"));
                headers.append(tr("BITS_PER_PIXEL"));
            } else if (format_key.contains("filters")){
                headers.append(tr("direct"));
            } else {
                headers.append(tr("detail"));
            }

            qDebug() << "header size:" << headers.size() << "head_code: " << head_code;

            // content
            auto tmp_content = parts.at(1).split("\n", QT_SKIP_EMPTY_PARTS);
            for (auto it : tmp_content) {
                QStringList tb_of_line;

                QStringList tmp = it.split(" ", QT_SKIP_EMPTY_PARTS);
                if (tmp.size() < 3){
                    continue;
                }
                for (int ch = 0; ch < head_code.size(); ++ch) {
                    tb_of_line.append("");
                }
                for (auto ch : tmp.at(0).trimmed()) {
                    int index = head_code.lastIndexOf(ch);

                    if (index >= 0) {
                        tb_of_line[index] = "√";
                    }
                }

                tb_of_line.append(tmp.at(1).trimmed());

                tmp.removeFirst();
                tmp.removeFirst();

                if (format_key.contains("pixfmts")) {
                    tb_of_line.append(tmp.at(0).trimmed());
                    tb_of_line.append(tmp.at(1).trimmed());
                }else {
                    tb_of_line.append(tmp.join(" "));
                }

                data_tb.append(tb_of_line);
            }
        }

        return;
    }

    // -colors -sample_fmts -layouts (inner: videorate, videosize)
    if (QStringList{"colors", "samplefmts", "layouts", "videorate", "videosize"}.contains(format_key, Qt::CaseInsensitive)) {
        headers << tr("Name") << "Value";

        for (int i = 1; i < rawStringLines.size(); ++i) {
            if (rawStringLines.at(i).contains(":", Qt::CaseInsensitive) ||
                rawStringLines.at(i).contains("DECOMPOSITION", Qt::CaseInsensitive) ||
                rawStringLines.at(i).contains("DESCRIPTION", Qt::CaseInsensitive)) {
                continue;
            }
            QStringList tmp = rawStringLines.at(i).split(" ", QT_SKIP_EMPTY_PARTS);
            if (tmp.size() >= 2) {
                QStringList tb_of_line{tmp.at(0).trimmed()};
                tmp.removeFirst();
                tb_of_line.append(tmp.join(" "));
                data_tb.append(tb_of_line);
            }
        }

        return;
    }

    // -protocols
    if (QStringList{"protocols"}.contains(format_key, Qt::CaseInsensitive)) {
        headers << tr("Input") << "Output";

        QStringList input, output;
        bool startOutput = false;
        for (int i = 2; i < rawStringLines.size(); i++) {
            if (rawStringLines.at(i).contains("output", Qt::CaseInsensitive)){
                startOutput = true;
                continue;
            }
            if (startOutput){
                output.append(rawStringLines.at(i).trimmed());
            } else {
                input.append(rawStringLines.at(i).trimmed());
            }
        }

        int maxRow = std::max(input.size(), output.size());

        for (int i = 0; i < maxRow; i++) {
            QStringList tmpRow;
            if (input.size() <= i) {
                tmpRow.append("");
            } else {
                tmpRow.append(input.at(i));
            }

            if (output.size() <= i) {
                tmpRow.append("");
            } else {
                tmpRow.append(output.at(i));
            }

            data_tb.append(tmpRow);
        }

        return;
    }

    // -bsfs -buildconf
    if (QStringList{"bsfs", "buildconf"}.contains(format_key, Qt::CaseInsensitive)) {
        headers.append(tr("name"));

        for (int i = 0; i < rawStringLines.size(); i++) {
            data_tb.append(QStringList{rawStringLines.at(i).trimmed()});
        }

        return;
    }

    if (format_key == "") {

    }
}

QString TableFormatWG::valueToString(const QJsonValue &value)
{
    if (value.isNull()) return "null";
    if (value.isUndefined()) return "undefined";
    if (value.isBool()) return value.toBool() ? "true" : "false";
    if (value.isDouble()) return QString::number(value.toDouble());
    if (value.isString()) return value.toString();
    if (value.isArray()) return QString("[Array(%1)]").arg(value.toArray().size());
    if (value.isObject()) return "{Object}";
    return value.toString();
}

QString TableFormatWG::extractSideData(const QJsonObject &frameObj, const QString &key)
{
    if (!frameObj.contains("side_data_list") || !frameObj["side_data_list"].isArray()) {
        return "";
    }

    QJsonArray sideDataArray = frameObj["side_data_list"].toArray();
    QStringList values;

    for (const QJsonValue &sideDataValue : sideDataArray) {
        if (sideDataValue.isObject()) {
            QJsonObject sideDataObj = sideDataValue.toObject();
            if (sideDataObj.contains(key)) {
                values.append(valueToString(sideDataObj[key]));
            }
        }
    }

    return values.join("; ");
}

QStringList TableFormatWG::getSelectedMediaTypes()
{

    QStringList mediaTypes;

    if (m_data_tb.isEmpty() || m_headers.isEmpty()) {
        return mediaTypes;
    }

    // Find the media_type column index
    int mediaTypeColumn = -1;
    for (int i = 0; i < m_headers.size(); ++i) {
        if (m_headers[i] == "media_type") {
            mediaTypeColumn = i;
            break;
        }
    }

    if (mediaTypeColumn == -1) {
        return mediaTypes; // media_type column not found
    }

    // Get selected rows from InfoWidgets
    QList<int> selectedRows = this->getSelectRows();

    if (selectedRows.isEmpty()) {
        // If no rows selected, check the first row
        if (!m_data_tb.isEmpty()) {
            selectedRows.append(0);
        }
    }

    // Extract media_type values from selected rows
    QSet<QString> uniqueTypes;
    for (int row : selectedRows) {
        if (row >= 0 && row < m_data_tb.size()) {
            const QStringList &rowData = m_data_tb[row];
            if (mediaTypeColumn < rowData.size()) {
                QString mediaType = rowData[mediaTypeColumn].trimmed();
                if (!mediaType.isEmpty()) {
                    uniqueTypes.insert(mediaType);
                }
            }
        }
    }

    mediaTypes = uniqueTypes.values();
    return mediaTypes;
}

void TableFormatWG::updateImageMenuVisibility()
{
    if (!m_imageMenu)
        return;

    QStringList mediaTypes = getSelectedMediaTypes();

    bool hasVideo = mediaTypes.contains("video") || mediaTypes.isEmpty();

    if (hasVideo) {
        addContextMenuMenu(m_imageMenu);
    } else {
        removeContextMenuMenu(m_imageMenu);
    }
}

void TableFormatWG::initDetailTb(const QString &data, const QString& format_key)
{
    formatData(data, m_data_tb, m_headers, format_key);

    initHeaderDetailTb(m_headers);
    updateDataDetailTb(m_data_tb);
}

void TableFormatWG::on_expand_raw_btn_clicked(bool checked)
{
    ui->detail_raw_pte->setVisible(checked);
}

void TableFormatWG::on_search_le_editingFinished()
{
    emit ui->search_btn->clicked();
}

void TableFormatWG::setupSearchButton()
{

}

void TableFormatWG::createDetailSearchDialog()
{
    if (!m_detailSearchDialog) {
        m_detailSearchDialog = new SearchWG(this);
        m_detailSearchDialog->setWindowTitle(tr("Detail Search"));
        m_detailSearchDialog->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
        // m_detailSearchDialog->setWindowModality(Qt::ApplicationModal);
        
        // Configure to show only the required group boxes for InfoWidgets
        auto requiredBoxes = SearchWG::SearchRange | SearchWG::MatchControl | SearchWG::Operation;
        m_detailSearchDialog->setVisibleGroupBoxes(requiredBoxes);
        
        // Set search range options based on current table headers
        if (!m_headers.isEmpty()) {
            m_detailSearchDialog->setSearchRangeOptions(m_headers);
        }
        
        // Connect signals
        connect(m_detailSearchDialog, &SearchWG::searchRangeSelectionChanged,
                this, &TableFormatWG::onDetailSearchCompleted);

        connect(m_detailSearchDialog, &SearchWG::searchReady,
                this, &TableFormatWG::onDetailSearchCompleted);

        connect(m_detailSearchDialog, &SearchWG::searchTextChanged, [=](QString text) {
            ui->search_le->setText(text);
        });

        // Resize dialog appropriately
        m_detailSearchDialog->adjustSize();
        m_detailSearchDialog->setMinimumWidth(400);
    }
}

void TableFormatWG::showDetailSearch()
{
    // Show search title
    ui->search_title_wg->setVisible(!ui->search_title_wg->isVisible());

    // Update search range options with current headers
    if (!m_headers.isEmpty()) {
        m_detailSearchDialog->setSearchRangeOptions(m_headers);
    }
    
    // Show the dialog
    m_detailSearchDialog->show();
    m_detailSearchDialog->raise();
    m_detailSearchDialog->activateWindow();
}

void TableFormatWG::onDetailSearchCompleted()
{
    // Get search parameters from SearchWG
    QStringList selectedRanges = m_detailSearchDialog->getSelectedSearchRanges();
    QString searchText = ui->search_le->text().trimmed();
    
    qDebug() << "Detail search completed with selected ranges:" << selectedRanges;
    qDebug() << "Search text:" << searchText;
    
    if (searchText.isEmpty()) {
        ui->detail_tb->setModel(multiColumnSearchModel);
        multiColumnSearchModel->resetFilters();
        updateCurrentModel();
        return;
    }
    
    // Configure multi-column search model
    multiColumnSearchModel->setSearchText(searchText);
    
    // Set match control options from SearchWG
    multiColumnSearchModel->setCaseSensitive(m_detailSearchDialog->isCaseSensitive());
    multiColumnSearchModel->setMatchWholeWords(m_detailSearchDialog->isMatchWholewords());
    multiColumnSearchModel->setUseRegularExpression(m_detailSearchDialog->isUseRegularExpression());
    
    // Configure search columns
    if (selectedRanges.isEmpty()) {
        // No columns selected, search in all columns
        multiColumnSearchModel->setSearchMode(false);
        qDebug() << "Searching in all columns";
    } else {
        // Search only in selected columns
        multiColumnSearchModel->setSearchColumns(selectedRanges);
        multiColumnSearchModel->setSearchMode(true);
        qDebug() << "Searching in selected columns:" << selectedRanges;
    }

    ui->detail_tb->setModel(multiColumnSearchModel);
    
    updateCurrentModel();
    
    qDebug() << "Multi-column search applied. Filtered rows:" << multiColumnSearchModel->rowCount();
}

void TableFormatWG::updateCurrentModel()
{
    m_headerManager->updateTotalCount(multiColumnSearchModel->rowCount());
}

QString TableFormatWG::getSelectedText(bool includeHeader)
{
    QModelIndexList selected = ui->detail_tb->selectionModel()->selectedIndexes();
    if (selected.isEmpty()) {
        return QString();
    }

    QString text;
    QItemSelectionRange range = ui->detail_tb->selectionModel()->selection().first();

    if (includeHeader) {
        QStringList headerContents;
        for (int j = range.left(); j <= range.right(); ++j) {
            int sourceColumn = j;
            if (ui->detail_tb->model() == multiColumnSearchModel) {
                sourceColumn = j;
            }
            
            if (sourceColumn >= 0 && sourceColumn < m_headers.size()) {
                headerContents << m_headers.at(sourceColumn);
            } else {
                headerContents << ui->detail_tb->model()->headerData(j, Qt::Horizontal).toString();
            }
        }
        text += headerContents.join("\t") + "\n";
    }

    for (int i = range.top(); i <= range.bottom(); ++i) {
        QStringList rowContents;
        for (int j = range.left(); j <= range.right(); ++j) {
            rowContents << ui->detail_tb->model()->index(i,j).data().toString();
        }
        text += rowContents.join("\t") + "\n";
    }

    return text;
}

void TableFormatWG::copyData(CopyOperation operation)
{
    QString progressMessage;
    switch (operation) {
    case CopySelectedText:
        progressMessage = tr("Preparing to copy selected text...");
        break;
    case CopySelectedTextWithHeader:
        progressMessage = tr("Preparing to copy selected text with header...");
        break;
    case CopySelectedRows:
        progressMessage = tr("Preparing to copy selected rows...");
        break;
    case CopySelectedRowsWithHeader:
        progressMessage = tr("Preparing to copy selected rows with headers...");
        break;
    case CopySelectedColumns:
        progressMessage = tr("Preparing to copy selected columns...");
        break;
    case CopySelectedColumnsWithHeader:
        progressMessage = tr("Preparing to copy selected columns with headers...");
        break;
    case CopyAllData:
        progressMessage = tr("Preparing to copy all data...");
        break;
    case CopyAllDataWithHeader:
        progressMessage = tr("Preparing to copy all data with headers...");
        break;
    }

    // Create progress dialog
    m_copyProgressDialog = new ProgressDialog(this);
    m_copyProgressDialog->setWindowTitle(tr("Copying Data"));
    m_copyProgressDialog->setMessage(progressMessage);
    m_copyProgressDialog->setProgressMode(ProgressDialog::Indeterminate);
    m_copyProgressDialog->setAutoClose(true);
    m_copyProgressDialog->setCancelButtonVisible(true);
    m_copyProgressDialog->start();
    
    // Use QtConcurrent::run for non-blocking operation
    QtConcurrent::run([this, operation]() {
        // Prepare text using the centralized function
        QString text = prepareCopyData(operation);
        
        if (!text.isEmpty()) {
            // Copy text directly
            QApplication::clipboard()->setText(text);

            // Update progress and finish in main thread
            QMetaObject::invokeMethod(this, [this]() {
                if (m_copyProgressDialog) {
                    m_copyProgressDialog->messageChanged(tr("Copy completed"));
                    m_copyProgressDialog->toFinish();
                    m_copyProgressDialog->deleteLater();
                    m_copyProgressDialog = nullptr;
                }
            }, Qt::QueuedConnection);
        } else {
            // Handle empty text case
            QMetaObject::invokeMethod(this, [this]() {
                if (m_copyProgressDialog) {
                    m_copyProgressDialog->toFinish();
                    m_copyProgressDialog->deleteLater();
                    m_copyProgressDialog = nullptr;
                }
            }, Qt::QueuedConnection);
        }
    });

    m_copyProgressDialog->exec();
}

QString TableFormatWG::prepareCopyData(CopyOperation operation)
{
    QString text;
    
    switch (operation) {
    case CopySelectedText:
        text = getSelectedText(false);
        break;
        
    case CopySelectedTextWithHeader:
        text = getSelectedText(true);
        break;
        
    case CopySelectedRows: {
        QSet<int> availableRows;
        QModelIndexList selectedIndexes = ui->detail_tb->selectionModel()->selectedIndexes();
        
        foreach (const QModelIndex &index, selectedIndexes) {
            auto sourceIndex = multiColumnSearchModel->mapToSource(index);
            availableRows.insert(sourceIndex.row());
        }
        
        if (availableRows.isEmpty()) {
            return QString();
        }
        
        QList<int> sortedRows = availableRows.values();
        std::sort(sortedRows.begin(), sortedRows.end());
        
        text.reserve(sortedRows.size() * m_headers.size() * 20); // Pre-allocate memory
        
        foreach (int row, sortedRows) {
            if (row >= 0 && row < m_data_tb.size()) {
                text += m_data_tb.at(row).join("\t") + "\n";
            }
        }
        break;
    }
        
    case CopySelectedRowsWithHeader: {
        QSet<int> availableRows;
        QModelIndexList selectedIndexes = ui->detail_tb->selectionModel()->selectedIndexes();
        
        foreach (const QModelIndex &index, selectedIndexes) {
            auto sourceIndex = multiColumnSearchModel->mapToSource(index);
            availableRows.insert(sourceIndex.row());
        }
        
        if (availableRows.isEmpty()) {
            return QString();
        }
        
        QList<int> sortedRows = availableRows.values();
        std::sort(sortedRows.begin(), sortedRows.end());
        
        text.reserve((sortedRows.size() + 1) * m_headers.size() * 20); // Pre-allocate memory
        
        // Add headers
        if (!m_headers.isEmpty()) {
            text += m_headers.join("\t") + "\n";
        }
        
        // Copy data in row order
        foreach (int row, sortedRows) {
            if (row >= 0 && row < m_data_tb.size()) {
                text += m_data_tb.at(row).join("\t") + "\n";
            }
        }
        break;
    }
        
    case CopySelectedColumns: {
        QModelIndexList selectedIndexes = ui->detail_tb->selectionModel()->selectedIndexes();
        if (selectedIndexes.isEmpty()) {
            return QString();
        }
        
        // Get all unique selected columns
        QSet<int> selectedColumns;
        foreach (const QModelIndex &index, selectedIndexes) {
            auto sourceIndex = multiColumnSearchModel->mapToSource(index);
            selectedColumns.insert(sourceIndex.column());
        }
        
        if (selectedColumns.isEmpty()) {
            return QString();
        }
        
        // Sort columns for consistent order
        QList<int> sortedColumns = selectedColumns.values();
        std::sort(sortedColumns.begin(), sortedColumns.end());
        
        text.reserve(m_data_tb.size() * sortedColumns.size() * 20); // Pre-allocate memory
        
        // Copy data for each selected column - optimized version
        for (int row = 0; row < m_data_tb.size(); ++row) {
            const QStringList &rowData = m_data_tb.at(row); // Use reference to avoid copying
            for (int i = 0; i < sortedColumns.size(); ++i) {
                if (i > 0) {
                    text += '\t';
                }
                int column = sortedColumns.at(i);
                if (column < rowData.size()) {
                    text += rowData.at(column);
                }
            }
            if (row < m_data_tb.size() - 1) {
                text += '\n';
            }
        }
        break;
    }
        
    case CopySelectedColumnsWithHeader: {
        QModelIndexList selectedIndexes = ui->detail_tb->selectionModel()->selectedIndexes();
        if (selectedIndexes.isEmpty()) {
            return QString();
        }
        
        // Get all unique selected columns
        QSet<int> selectedColumns;
        foreach (const QModelIndex &index, selectedIndexes) {
            auto sourceIndex = multiColumnSearchModel->mapToSource(index);
            selectedColumns.insert(sourceIndex.column());
        }
        
        if (selectedColumns.isEmpty()) {
            return QString();
        }
        
        // Sort columns for consistent order
        QList<int> sortedColumns = selectedColumns.values();
        std::sort(sortedColumns.begin(), sortedColumns.end());
        
        text.reserve(m_headers.size() + m_data_tb.size() * sortedColumns.size() * 20); // Pre-allocate memory
        
        // Add headers for selected columns - optimized version
        for (int i = 0; i < sortedColumns.size(); ++i) {
            if (i > 0) {
                text += '\t';
            }
            int column = sortedColumns.at(i);
            if (column < m_headers.size()) {
                text += m_headers.at(column);
            }
        }
        text += '\n';
        
        // Copy data for each selected column - optimized version
        for (int row = 0; row < m_data_tb.size(); ++row) {
            const QStringList &rowData = m_data_tb.at(row); // Use reference
            for (int i = 0; i < sortedColumns.size(); ++i) {
                if (i > 0) {
                    text += '\t';
                }
                int column = sortedColumns.at(i);
                if (column < rowData.size()) {
                    text += rowData.at(column);
                }
            }
            if (row < m_data_tb.size() - 1) {
                text += '\n';
            }
        }
        break;
    }
        
    case CopyAllData: {
        text.reserve(m_data_tb.size() * m_headers.size() * 20); // Pre-allocate memory
        
        for (const QStringList &row : m_data_tb) {
            text += row.join("\t") + "\n";
        }
        break;
    }
        
    case CopyAllDataWithHeader: {
        text.reserve((m_data_tb.size() + 1) * m_headers.size() * 20); // Pre-allocate memory

        // Add headers
        if (!m_headers.isEmpty()) {
            text += m_headers.join("\t") + "\n";
        }

        // Add all data
        for (const QStringList &row : m_data_tb) {
            text += row.join("\t") + "\n";
        }
        break;
    }
    }
    
    return text;
}

void TableFormatWG::on_search_le_textChanged(const QString &arg1)
{
    if (m_detailSearchDialog) {
        m_detailSearchDialog->setSearchText(arg1);
    }
}

void TableFormatWG::copySelectedText()
{
    copyData(CopySelectedText);
}

void TableFormatWG::copySelectedTextWithHeader()
{
    copyData(CopySelectedTextWithHeader);
}

void TableFormatWG::copySelectedRows()
{
    copyData(CopySelectedRows);
}

void TableFormatWG::copySelectedRowsWithHeader()
{
    copyData(CopySelectedRowsWithHeader);
}

void TableFormatWG::copySelectedColumns()
{
    copyData(CopySelectedColumns);
}

void TableFormatWG::copySelectedColumnsWithHeader()
{
    copyData(CopySelectedColumnsWithHeader);
}

void TableFormatWG::copyAllData()
{
    copyData(CopyAllData);
}

void TableFormatWG::copyAllDataWithHeader()
{
    copyData(CopyAllDataWithHeader);
}

void TableFormatWG::fitTableColumnToContent()
{
    QTimer::singleShot(50, this, [this]() {
        resizeColumnsProportionally();
    });
}

void TableFormatWG::showDetailInfo()
{
    QModelIndex currentIndex = ui->detail_tb->currentIndex();

    int columnIndex = m_headers.indexOf("name");
    int rowIndex = -1;
    auto index = multiColumnSearchModel->mapToSource(currentIndex);

    if (m_helpKey == PROTOCOL_FMT) {
        columnIndex = index.column();
    }
    rowIndex = index.row();

    if (columnIndex < 0 || rowIndex < 0) {
        qCritical() << "can't find selected column or row" << columnIndex << rowIndex;
        return;
    }

    qDebug() << "current name-s: " << m_data_tb[rowIndex][columnIndex];

    HelpQueryWg *helpWindow = new HelpQueryWg;
    helpWindow->setAttribute(Qt::WA_DeleteOnClose);
    helpWindow->setControlHeaderVisiable(false);

    helpWindow->setWindowTitle(tr("Help Query %1=%2").arg(m_helpKey).arg(m_data_tb[rowIndex][columnIndex]));
    helpWindow->setHelpParams(m_helpKey, m_data_tb[rowIndex][columnIndex]);
    helpWindow->show();
    ZWindowHelper::centerToParent(helpWindow);
}

void TableFormatWG::setupColumnWidthManagement()
{
    // Setup resize timer for performance optimization
    // m_resizeTimer->setSingleShot(true);
    m_resizeTimer->setInterval(100); // 100ms delay
    connect(m_resizeTimer, &QTimer::timeout, this, &TableFormatWG::onResizeTimerTimeout);
    
    // Connect header resize signal
    connect(ui->detail_tb->horizontalHeader(), &QHeaderView::sectionResized,
            this, &TableFormatWG::onHeaderSectionResized);
    
    // Install event filter on table view to catch resize events
    ui->detail_tb->installEventFilter(this);
}

void TableFormatWG::saveColumnWidthRatios()
{
    if (m_headers.isEmpty() || !ui->detail_tb->horizontalHeader()) {
        return;
    }
    
    QHeaderView *header = ui->detail_tb->horizontalHeader();
    int totalWidth = 0;
    m_columnWidthRatios.clear();
    
    // Calculate total width
    for (int i = 0; i < m_headers.size(); ++i) {
        totalWidth += header->sectionSize(i);
    }
    
    if (totalWidth <= 0) {
        return;
    }
    
    // Calculate ratios
    for (int i = 0; i < m_headers.size(); ++i) {
        double ratio = static_cast<double>(header->sectionSize(i)) / totalWidth;
        m_columnWidthRatios.append(ratio);
    }
    
    m_lastTableWidth = ui->detail_tb->viewport()->width();
    // qDebug() << "Saved column width ratios:" << m_columnWidthRatios;
}

void TableFormatWG::restoreColumnWidthRatios()
{
    if (m_columnWidthRatios.isEmpty() || m_headers.isEmpty() || !ui->detail_tb->horizontalHeader()) {
        return;
    }
    
    QHeaderView *header = ui->detail_tb->horizontalHeader();
    int availableWidth = ui->detail_tb->viewport()->width();

    if (availableWidth <= 0) {
        return;
    }
    
    // Temporarily disable stretch mode
    header->setSectionResizeMode(QHeaderView::Interactive);
    
    // Apply ratios
    for (int i = 0; i < qMin(m_columnWidthRatios.size(), m_headers.size()); ++i) {
        int newWidth = static_cast<int>(availableWidth * m_columnWidthRatios[i]);
        newWidth = qMax(newWidth, 50); // Minimum width
        header->resizeSection(i, newWidth);
    }
    
    m_lastTableWidth = availableWidth;
}

void TableFormatWG::resizeColumnsProportionally()
{
    if (!m_isUserAdjusted || m_columnWidthRatios.isEmpty() || m_headers.isEmpty()) {
        setupInitialColumnWidths();
        return;
    }

    restoreColumnWidthRatios();
}

void TableFormatWG::setupInitialColumnWidths()
{
    if (m_headers.isEmpty() || !ui->detail_tb->horizontalHeader()) {
        return;
    }
    
    QHeaderView *header = ui->detail_tb->horizontalHeader();
    
    // If user hasn't adjusted columns, use automatic sizing
    if (!m_isUserAdjusted) {
        header->setSectionResizeMode(QHeaderView::ResizeToContents);
        
        // Process events to ensure content-based sizing is applied
        QApplication::processEvents();
        
        // Set last column to stretch if there are multiple columns
        if (m_headers.size() > 1) {
            header->setSectionResizeMode(m_headers.size() - 1, QHeaderView::Stretch);
        }
        
        // Save initial ratios after auto-sizing
        QTimer::singleShot(50, this, [this]() {
            saveColumnWidthRatios();
        });
    } else {
        // Restore user-adjusted proportions
        restoreColumnWidthRatios();
    }
}

void TableFormatWG::onHeaderSectionResized(int logicalIndex, int oldSize, int newSize)
{
    Q_UNUSED(logicalIndex)
    Q_UNUSED(oldSize)
    Q_UNUSED(newSize)
    
    // Mark as user-adjusted when user manually resizes columns
    if (!m_isUserAdjusted) {
        m_isUserAdjusted = true;
        qDebug() << "Column widths marked as user-adjusted";
    }
    
    // Use timer to avoid frequent updates during dragging
    m_resizeTimer->start();
}

void TableFormatWG::onResizeTimerTimeout()
{
    // Save new ratios after user adjustment
    saveColumnWidthRatios();
}

void TableFormatWG::previewImage()
{

    // Get the current media file from settings
    Common *common = Common::instance();
    QString currentFile = common->getConfigValue(CURRENTFILE).toString();

    if (currentFile.isEmpty()) {
        QMessageBox::warning(this, "Preview Error", "No media file selected for preview.");
        return;
    }

    // Ensure the save directory exists using Common class method
    if (!Common::ensureDirectory(DEFAULT_IMAGE_PREVIEW_PATH)) {
        QMessageBox::warning(this, "Preview Error",
                             QString("Failed to create preview directory: %1").arg(DEFAULT_IMAGE_PREVIEW_PATH));
        return;
    }

    QDir saveDir(DEFAULT_IMAGE_PREVIEW_PATH);

    // Get selected rows using InfoWidgets::getSelectRows()
    QList<int> selectedRows = this->getSelectRows();

    // If no rows selected, use the first row
    if (selectedRows.isEmpty()) {
        if (m_data_tb.isEmpty()) {
            QMessageBox::information(this, "Preview Info",
                                     "No frame data available for preview.");
            return;
        }
        selectedRows.append(0);
    }

    // Create progress dialog for preview
    ProgressDialog *progressDialog = new ProgressDialog(this);
    progressDialog->setWindowTitle(tr("Extracting Images for Preview"));
    progressDialog->setProgressMode(ProgressDialog::Determinate);
    progressDialog->setRange(0, selectedRows.size());
    progressDialog->setAutoClose(false);
    progressDialog->setCancelButtonVisible(false);
    progressDialog->show();

    int successCount = 0;
    int totalCount = selectedRows.size();

    // Process each selected row
    for (int i = 0; i < selectedRows.size(); ++i) {
        int selectedRow = selectedRows[i];

        if (selectedRow < 0 || selectedRow >= m_data_tb.size()) {
            qWarning() << "Invalid row index:" << selectedRow;
            continue;
        }

        // Update progress
        progressDialog->setValue(i);
        progressDialog->setMessage(tr("Extracting frame %1 of %2 for preview...").arg(i + 1).arg(totalCount));
        QApplication::processEvents(); // Ensure UI updates

        int frameNumber = selectedRow;

        // Generate output filename with timestamp for unique identification
        QFileInfo fileInfo(currentFile);
        QString baseName = fileInfo.baseName();

        // Create a subdirectory for the video file
        QDir videoDir(saveDir.absoluteFilePath(baseName));
        if (!videoDir.exists()) {
            if (!videoDir.mkpath(".")) {
                QMessageBox::warning(this, "Preview Error",
                                     QString("Failed to create video directory: %1").arg(videoDir.absolutePath()));
                continue;
            }
        }

        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
        QString outputFileName = QString("%1_frame_%2_%3.jpg").arg(baseName).arg(frameNumber).arg(timestamp);
        QString outputFilePath = videoDir.absoluteFilePath(outputFileName);

        // Extract the frame using ffmpeg
        ZFFmpeg ffmpeg;
        if (!ffmpeg.extractFrame(currentFile, frameNumber, outputFilePath)) {
            qWarning() << "Failed to extract frame" << frameNumber << "for preview";
            continue;
        }

        // Verify the extracted image exists
        if (!QFile::exists(outputFilePath)) {
            qWarning() << "Failed to create the extracted image file for frame" << frameNumber;
            continue;
        }

        // Use zffplay to display the extracted image with size limits
        ZFFplay *ffplay = new ZFFplay(this);
        if (!ffplay->displayImageWithSize(outputFilePath, 800, 600, frameNumber, baseName)) {
            qWarning() << "Failed to display the extracted image with ffplay for frame" << frameNumber;
            delete ffplay;
            continue;
        }

        // Connect signal to clean up the ffplay instance when playback finishes
        connect(ffplay, &ZFFplay::playbackFinished, [ffplay]() {
            ffplay->deleteLater();
        });

        connect(ffplay, &ZFFplay::errorOccurred, [ffplay](const QString &error) {
            qWarning() << "ZFFplay error:" << error;
            ffplay->deleteLater();
        });

        successCount++;
        qDebug() << "TabelFormatWG: Extracted frame" << frameNumber
                 << "saved to" << outputFilePath << "and opened with ffplay";
    }

    // Finish progress
    progressDialog->setValue(totalCount);
    progressDialog->setMessage(tr("Completed. Extracted %1 of %2 images for preview.").arg(successCount).arg(totalCount));
    progressDialog->finish();

    // Clean up progress dialog after a short delay
    QTimer::singleShot(2000, [progressDialog]() {
        progressDialog->deleteLater();
    });
}

void TableFormatWG::saveImage()
{

    // Get the current media file from settings
    Common *common = Common::instance();
    QString currentFile = common->getConfigValue(CURRENTFILE).toString();

    if (currentFile.isEmpty()) {
        QMessageBox::warning(this, "Save Image Error", "No media file selected for saving.");
        return;
    }

    // Get selected rows using InfoWidgets::getSelectRows()
    QList<int> selectedRows = this->getSelectRows();

    // If no rows selected, use the first row
    if (selectedRows.isEmpty()) {
        if (m_data_tb.isEmpty()) {
            QMessageBox::information(this, "Save Image Info",
                                     "No frame data available for saving.");
            return;
        }
        selectedRows.append(0);
    }

    // Let user select a directory to save images
    QString saveDir = QFileDialog::getExistingDirectory(this,
                                                        tr("Select Directory to Save Images"),
                                                        QDir::homePath(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (saveDir.isEmpty()) {
        // User cancelled the dialog
        return;
    }

    // Create progress dialog
    ProgressDialog *progressDialog = new ProgressDialog(this);
    progressDialog->setWindowTitle(tr("Saving Images"));
    progressDialog->setProgressMode(ProgressDialog::Determinate);
    progressDialog->setRange(0, selectedRows.size());
    progressDialog->setAutoClose(false);
    progressDialog->setCancelButtonVisible(false);
    progressDialog->show();

    // Create a subdirectory for the video file
    QFileInfo fileInfo(currentFile);
    QString baseName = fileInfo.baseName();
    QDir videoDir(QDir(saveDir).absoluteFilePath(baseName));
    if (!videoDir.exists()) {
        if (!videoDir.mkpath(".")) {
            QMessageBox::warning(this, "Save Image Error",
                                 QString("Failed to create video directory: %1").arg(videoDir.absolutePath()));
            progressDialog->finish();
            progressDialog->deleteLater();
            return;
        }
    }

    int successCount = 0;
    int totalCount = selectedRows.size();

    // Process each selected row
    for (int i = 0; i < selectedRows.size(); ++i) {
        int selectedRow = selectedRows[i];

        if (selectedRow < 0 || selectedRow >= m_data_tb.size()) {
            qWarning() << "Invalid row index:" << selectedRow;
            continue;
        }

        // Update progress
        progressDialog->setValue(i);
        progressDialog->setMessage(tr("Saving frame %1 of %2...").arg(i + 1).arg(totalCount));
        QApplication::processEvents(); // Ensure UI updates

        int frameNumber = selectedRow;

        // Generate output filename
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
        QString outputFileName = QString("%1_frame_%2_%3.jpg").arg(baseName).arg(frameNumber).arg(timestamp);
        QString outputFilePath = videoDir.absoluteFilePath(outputFileName);

        // Extract the frame using ffmpeg
        ZFFmpeg ffmpeg;
        if (!ffmpeg.extractFrame(currentFile, frameNumber, outputFilePath)) {
            qWarning() << "Failed to extract frame" << frameNumber;
            continue;
        }

        // Verify the extracted image exists
        if (!QFile::exists(outputFilePath)) {
            qWarning() << "Failed to create the extracted image file for frame" << frameNumber;
            continue;
        }

        successCount++;
        qDebug() << "TabelFormatWG: Extracted frame" << frameNumber
                 << "saved to" << outputFilePath;
    }

    // Finish progress
    progressDialog->setValue(totalCount);
    progressDialog->setMessage(tr("Completed. Saved %1 of %2 images.").arg(successCount).arg(totalCount));
    progressDialog->finish();

    // Show completion message and ask if user wants to open folder
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Save Complete"),
                                  tr("Successfully saved %1 of %2 images to:\n%3\n\nDo you want to open the folder?")
                                      .arg(successCount)
                                      .arg(totalCount)
                                      .arg(videoDir.absolutePath()),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Open the directory containing the saved images
        QDesktopServices::openUrl(QUrl::fromLocalFile(videoDir.absolutePath()));
    }

    // Clean up progress dialog
    progressDialog->deleteLater();
}

void TableFormatWG::onContextMenuAboutToShow()
{
    updateImageMenuVisibility();
}

bool TableFormatWG::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->detail_tb && event->type() == QEvent::Resize) {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event);
        int newWidth = resizeEvent->size().width();
        
        // Only trigger proportional resize if width changed significantly
        if (qAbs(newWidth - m_lastTableWidth) > 10) {
            // Use timer to avoid frequent resizing during window drag
            QTimer::singleShot(50, this, [this]() {
                resizeColumnsProportionally();
            });
        }
    }
    
    return QWidget::eventFilter(obj, event);
}

bool TableFormatWG::loadJson(const QByteArray &json)
{

    qDebug() << "here table";

    m_headers.clear();
    m_data_tb.clear();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "JSON is not an object";
        return false;
    }

    QJsonObject rootObject = doc.object();

    QStringList availableKeys{"frames", "packets"};

    QString key;
    for (auto it : availableKeys) {
        if (!rootObject.contains(it) || !rootObject[it].isArray()) {
            qWarning() << "Missing or invalid " << it << " array";
        } else {
            key = it;
        }
    }

    if (key.isEmpty()) return false;

    QJsonArray framesArray = rootObject[key].toArray();

    if (framesArray.isEmpty()) {
        qDebug() << "Empty frames array";
        return true;
    }

    // Field classification
    struct FieldCategory {
        QStringList common;
        QStringList video;
        QStringList audio;
        QSet<QString> allFields;
    };

    FieldCategory categories = {
        // Common fields
        {
            "media_type", "stream_index", "key_frame", "pkt_pts", "pkt_pts_time",
            "pkt_dts", "pkt_dts_time", "best_effort_timestamp", "best_effort_timestamp_time",
            "pkt_duration", "pkt_duration_time", "pkt_pos", "pkt_size"
        },
        // Video-specific fields
        {
            "width", "height", "pix_fmt", "sample_aspect_ratio", "pict_type",
            "coded_picture_number", "display_picture_number", "interlaced_frame",
            "top_field_first", "repeat_pict", "chroma_location"
        },
        // Audio-specific fields
        {
            "sample_fmt", "nb_samples", "channels", "channel_layout"
        },
        // All fields set
        {}
    };

    // Collect all fields
    for (const QJsonValue &frameValue : framesArray) {
        if (frameValue.isObject()) {
            QJsonObject obj = frameValue.toObject();
            for (const QString &key : obj.keys()) {
                categories.allFields.insert(key);
            }
        }
    }

    // Build optimized column order for each media type
    QHash<QString, QStringList> headerTemplates;

    auto buildHeader = [&](const QStringList& specificFields) {
        QStringList header;
        // Add common fields that exist in data
        for (const QString &field : categories.common) {
            if (categories.allFields.contains(field)) {
                header.append(field);
            }
        }
        // Add specific fields that exist in data
        for (const QString &field : specificFields) {
            if (categories.allFields.contains(field)) {
                header.append(field);
            }
        }
        // Add remaining fields in sorted order
        QSet<QString> otherFields = categories.allFields;
        for (const QString &field : header) {
            otherFields.remove(field);
        }
        QStringList sortedOther = otherFields.values();
        std::sort(sortedOther.begin(), sortedOther.end());
        header.append(sortedOther);
        return header;
    };

    headerTemplates["video"] = buildHeader(categories.video);
    headerTemplates["audio"] = buildHeader(categories.audio);
    headerTemplates["default"] = buildHeader({});

    // Value converter
    auto toString = [](const QJsonValue &val) -> QString {
        if (val.isNull()) return "null";
        if (val.isBool()) return val.toBool() ? "true" : "false";
        if (val.isDouble()) return QString::number(val.toDouble());
        if (val.isString()) return val.toString();
        if (val.isArray()) return QString("[%1 items]").arg(val.toArray().size());
        if (val.isObject()) return "{object}";
        return val.toString();
    };

    // Parse data
    QString currentMediaType;
    for (const QJsonValue &frameValue : framesArray) {
        if (!frameValue.isObject()) continue;

        QJsonObject frameObj = frameValue.toObject();
        QString mediaType = frameObj.contains("media_type") ?
                                frameObj["media_type"].toString() : "default";

        // Dynamically adjust column order
        if (m_headers.isEmpty() || mediaType != currentMediaType) {
            if (headerTemplates.contains(mediaType)) {
                m_headers = headerTemplates[mediaType];
            } else if (headerTemplates.contains("default")) {
                m_headers = headerTemplates["default"];
            } else {
                m_headers = categories.common + categories.allFields.values();
            }
            currentMediaType = mediaType;
        }

        // Extract row data
        QStringList rowData;
        for (const QString &column : m_headers) {
            rowData.append(frameObj.contains(column) ?
                               toString(frameObj[column]) : "");
        }
        m_data_tb.append(rowData);
    }

    qDebug() << "Parsed" << m_data_tb.size() << "frames with" << m_headers.size() << "columns";

    this->initHeaderDetailTb(m_headers, ", ");
    this->updateDataDetailTb(m_data_tb, ", ");

    return true;
}

void TableFormatWG::showContextMenu(const QPoint &pos)
{
    BaseFormatWG::showContextMenu(pos);
}

QModelIndexList TableFormatWG::getSelectedIndexes()
{
    // Get selected rows from InfoWidgets
    QList<int> selectedRows = this->getSelectRows();
    QModelIndexList indexes;

    // Convert row indices to model indexes
    for (int row : selectedRows) {
        if (row >= 0 && row < m_data_tb.size()) {
            // Create a dummy index for the first column
            // Since we can't access the model directly, create an invalid index
            // This is sufficient for our use case as we mainly use this to check if selection is empty
            indexes.append(QModelIndex());
        }
    }

    return indexes;
}

bool TableFormatWG::hasValidSelection()
{
    return !this->getSelectRows().isEmpty();
}

QString TableFormatWG::getAllData()
{
    // Convert table data to string format
    QString result;

    // Add headers
    if (!m_headers.isEmpty()) {
        result += m_headers.join("\t") + "\n";
    }

    // Add data rows
    for (const QStringList &row : m_data_tb) {
        result += row.join("\t") + "\n";
    }

    return result;
}

void TableFormatWG::setupTableModel()
{
    // model
    m_model = new MediaInfoTabelModel(this);

    multiColumnSearchModel = new MultiColumnSearchProxyModel(this);
    multiColumnSearchModel->setSourceModel(m_model);
    ui->detail_tb->setModel(multiColumnSearchModel);

    ui->detail_tb->horizontalHeader()->setSectionsMovable(true);
    ui->detail_tb->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->detail_tb->verticalHeader()->setDefaultSectionSize(25);
    ui->detail_tb->verticalHeader()->setVisible(true);
    ui->detail_tb->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->detail_tb->setSortingEnabled(true);

    // header manager
    m_headerManager = new ZTableHeaderManager(ui->detail_tb->horizontalHeader(), ui->detail_tb->verticalHeader(), this);
    m_headerManager->setObjectName(this->objectName());
    m_headerManager->setTotalCountVisible(false);
    m_headerManager->restoreState();

    connect(m_headerManager, &ZTableHeaderManager::headerToggleVisiable, [=]() {
        fitTableColumnToContent();
    });
}

