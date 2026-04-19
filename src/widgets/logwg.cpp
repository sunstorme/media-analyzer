// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "logwg.h"
#include "ui_logwg.h"
#include "common/zsyntaxhighlighter.h"
#include <QMenu>
#include <QShortcut>
#include <model/logmodel.h>

LogWG::LogWG(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LogWG)
    , m_logModel(new LogModel(this))
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->log_text_wg);
    
    // Enable log level syntax highlighting
    ui->log_ple->setSyntaxMode(ZHighlightMode::Log);
    
    // Setup log table view
    ui->log_tbv->setModel(m_logModel);
    ui->log_tbv->setAlternatingRowColors(true);
    ui->log_tbv->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    // Initialize header manager
    ui->log_tbv->horizontalHeader()->setSectionsMovable(true);
    ui->log_tbv->verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->log_tbv->verticalHeader()->setDefaultSectionSize(25);
    ui->log_tbv->verticalHeader()->setVisible(true);

    m_headerManager = new ZTableHeaderManager(ui->log_tbv->horizontalHeader(), ui->log_tbv->verticalHeader(), this);
    m_headerManager->setObjectName(this->objectName());
    m_headerManager->enableHeaderContextMenu(true);
    m_headerManager->setTotalCountVisible(false);

    m_highLighter = new ZTextHighlighter(ui->log_ple);

    m_searchWG = new SearchWG(this);
    m_searchWG->setWindowTitle(tr("Detail Search"));

    // Configure to show only the required group boxes for InfoWidgets
    auto requiredBoxes = SearchWG::MatchControl | SearchWG::Operation;
    m_searchWG->setVisibleGroupBoxes(requiredBoxes);

    ui->mainVerticalLayout->addWidget(m_searchWG);
    m_searchWG->setVisible(false);

    connect(m_searchWG, &SearchWG::searchReady, this, &LogWG::on_searchReady);
    connect(m_searchWG, &SearchWG::matchControlChanged, this, &LogWG::on_searchReady);
    connect(m_searchWG, &SearchWG::searchClear, this, [this]() {
        m_highLighter->clearHighlight();
        m_searchWG->setSearchText("");
        m_searchWG->setSearchStatus("");
    });
    connect(m_searchWG, &SearchWG::searchBefore, m_highLighter, &ZTextHighlighter::gotoPreviousHighlight);
    connect(m_searchWG, &SearchWG::searchNext, m_highLighter, &ZTextHighlighter::gotoNextHighlight);

    connect(m_highLighter, &ZTextHighlighter::highlightCountChanged, [=](int count) {
        m_searchWG->setSearchStatus(QString("Found %1 results").arg(count));
    });
    connect(m_highLighter, &ZTextHighlighter::currentHighlightChanged, [=](int index) {
        if (index >= 0) {
            m_searchWG->setSearchStatus(QString("Result %1 of %2").arg(index + 1).arg(m_highLighter->highlightCount()));
        }
    });
    connect(m_highLighter, &ZTextHighlighter::searchTextNotFound, [=](const QString &searchText) {
        m_searchWG->setSearchStatus(QString("Text '%1' not found").arg(searchText));
    });

    // Setup context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &LogWG::customContextMenuRequested, this, &LogWG::showContextMenu);

    // Setup shortcuts
    new QShortcut(QKeySequence("Ctrl+F"), this, SLOT(toggleSearchDetail()));
    new QShortcut(QKeySequence("Ctrl+T"), this, SLOT(toggleView()));
    
    // Restore header state
    m_headerManager->restoreState();
}

void LogWG::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);
    
    // Add search detail action
    QAction *searchAction = contextMenu.addAction(tr("Toggle Search Detail (Ctrl+F)"));
    connect(searchAction, &QAction::triggered, this, &LogWG::toggleSearchDetail);
    
    // Add view toggle action
    QAction *viewAction = contextMenu.addAction(tr("Toggle View (Ctrl+T)"));
    connect(viewAction, &QAction::triggered, this, &LogWG::toggleView);
    
    contextMenu.addSeparator();
    
    // Add clear logs action
    QAction *clearAction = contextMenu.addAction(tr("Clear Logs"));
    connect(clearAction, &QAction::triggered, this, [this](){
        ui->log_ple->clear();
        m_logModel->clearLogs();
    });
    
    contextMenu.exec(mapToGlobal(pos));
}

void LogWG::toggleSearchDetail()
{
    m_searchWG->setVisible(!m_searchWG->isVisible());
}

void LogWG::toggleView()
{
    // Toggle between text and table views
    if (ui->stackedWidget->currentWidget() == ui->log_text_wg) {
        ui->stackedWidget->setCurrentWidget(ui->log_table_wg);
    } else {
        ui->stackedWidget->setCurrentWidget(ui->log_text_wg);
    }
}

void LogWG::on_searchReady()
{
    QString searchText = m_searchWG->getSearchText().trimmed();
    if (searchText.isEmpty()) {
        m_searchWG->setSearchStatus(tr("Search text is empty"));
        return;
    }

    // Check if there's content to search
    if (ui->log_ple->toPlainText().isEmpty()) {
        m_searchWG->setSearchStatus(tr("No content to search"));
        return;
    }

    m_highLighter->setCaseSensitive(m_searchWG->isCaseSensitive());
    m_highLighter->setWholeWord(m_searchWG->isMatchWholewords());
    m_highLighter->setUseRegex(m_searchWG->isUseRegularExpression());

    m_highLighter->highlight(searchText);
}

LogWG::~LogWG()
{
    delete m_headerManager;
    delete ui;
}

void LogWG::outLog(const QString &log)
{
    // Add to text view
    ui->log_ple->appendPlainText(log);
    
    // Add to table model
    m_logModel->addLogEntry(log);
    
    // Update total count
    m_headerManager->updateTotalCount(m_logModel->rowCount());
    
    // Auto-scroll to bottom if table view is visible
    if (ui->stackedWidget->currentWidget() == ui->log_table_wg) {
        ui->log_tbv->scrollToBottom();
    }

    ui->log_tbv->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Time
    ui->log_tbv->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Level
    ui->log_tbv->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive); // Function (manual adjust)
    ui->log_tbv->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch); // Info (Auto Stretch)
}


