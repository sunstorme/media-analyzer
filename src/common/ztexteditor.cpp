// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "ztexteditor.h"
#include "common.h"
#include "zhighlightconfig.h"
#include "ztexthighlighter.h"
#include "widgets/searchwg.h"
#include <QPainter>
#include <QTextBlock>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QClipboard>
#include <QApplication>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPalette>
#include <QColor>

ZTextEditor::ZTextEditor(QWidget *parent)
    : QPlainTextEdit(parent)
{
    m_lineNumberArea = new ZLineNumberArea(this);

    // Connect signals and slots
    connect(this, &ZTextEditor::blockCountChanged,
            this, &ZTextEditor::updateLineNumberAreaWidth);
    connect(this, &ZTextEditor::updateRequest,
            this, &ZTextEditor::updateLineNumberArea);
    connect(this, &ZTextEditor::cursorPositionChanged,
            this, &ZTextEditor::highlightCurrentLine);
    // Listen for font changes to ensure line numbers match text font
    connect(this, &::ZTextEditor::textChanged,
            this, &ZTextEditor::updateLineNumberAreaWidth);

    updateLineNumberAreaWidth();
    highlightCurrentLine();
    
    // Setup context menu
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setupContextMenu();
    
    // Setup search functionality (disabled by default)
    setupSearch();
    
    // Setup syntax highlighting
    setupSyntaxHighlighter();
}

int ZTextEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        digits++;
    }
    // Add right margin when calculating width
    return 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits + ZHighlightConfig::Editor::lineNumberRightMargin();
}

void ZTextEditor::updateLineNumberAreaWidth()
{
    int bottomMargin = (m_searchWG && m_searchWG->isVisible()) ? m_searchWG->height() : 0;
    setViewportMargins(lineNumberAreaWidth(), 0, 0, bottomMargin);
}

void ZTextEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth();
}

void ZTextEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    
    // Position search widget at the bottom
    if (m_searchWG && m_searchWG->isVisible()) {
        int searchHeight = m_searchWG->height();
        m_searchWG->setGeometry(0, height() - searchHeight, width(), searchHeight);
    }
}

void ZTextEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        // Current line background color (GitHub light gray)
        selection.format.setBackground(ZHighlightConfig::Editor::currentLineBackground());
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

void ZTextEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    // Line number area background color (GitHub-style light gray)
    painter.fillRect(event->rect(), ZHighlightConfig::Editor::lineNumberBackground());

    // Use the same font as the editor for line numbers to ensure height matching
    painter.setFont(font());

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    // Get actual top position of text block (considering font height)
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    // Calculate text block height (based on actual font metrics)
    int blockHeight = fontMetrics().height();
    int bottom = top + blockHeight;

    int currentLine = textCursor().blockNumber();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            // Highlight current line number (GitHub blue)
            if (blockNumber == currentLine) {
                painter.setPen(ZHighlightConfig::Editor::currentLineNumber());
                painter.setFont(QFont(font().family(), font().pointSize(), QFont::Medium));
            } else {
                painter.setPen(ZHighlightConfig::Editor::lineNumberText());
                painter.setFont(font());
            }
            // Draw line numbers, accounting for right margin
            painter.drawText(0, top, m_lineNumberArea->width() - ZHighlightConfig::Editor::lineNumberRightMargin(),
                             blockHeight, Qt::AlignRight | Qt::AlignTop, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + blockHeight;
        blockNumber++;
    }
}

void ZTextEditor::contextMenuEvent(QContextMenuEvent *event)
{
    updateContextMenuActions();
    m_contextMenu->exec(event->globalPos());
}

void ZTextEditor::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    // Undo
    m_undoAction = new QAction(tr("Undo"), this);
    m_undoAction->setShortcut(QKeySequence::Undo);
    connect(m_undoAction, &QAction::triggered, this, &ZTextEditor::undo);
    m_contextMenu->addAction(m_undoAction);
    
    // Redo
    m_redoAction = new QAction(tr("Redo"), this);
    m_redoAction->setShortcut(QKeySequence::Redo);
    connect(m_redoAction, &QAction::triggered, this, &ZTextEditor::redo);
    m_contextMenu->addAction(m_redoAction);
    
    m_contextMenu->addSeparator();
    
    // Cut
    m_cutAction = new QAction(tr("Cut"), this);
    m_cutAction->setShortcut(QKeySequence::Cut);
    connect(m_cutAction, &QAction::triggered, this, &ZTextEditor::cut);
    m_contextMenu->addAction(m_cutAction);
    
    // Copy
    m_copyAction = new QAction(tr("Copy"), this);
    m_copyAction->setShortcut(QKeySequence::Copy);
    connect(m_copyAction, &QAction::triggered, this, &ZTextEditor::copy);
    m_contextMenu->addAction(m_copyAction);
    
    // Paste
    m_pasteAction = new QAction(tr("Paste"), this);
    m_pasteAction->setShortcut(QKeySequence::Paste);
    connect(m_pasteAction, &QAction::triggered, this, &ZTextEditor::paste);
    m_contextMenu->addAction(m_pasteAction);
    
    // Delete
    m_deleteAction = new QAction(tr("Delete"), this);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    connect(m_deleteAction, &QAction::triggered, this, &ZTextEditor::deleteSelected);
    m_contextMenu->addAction(m_deleteAction);
    
    m_contextMenu->addSeparator();
    
    // Select All
    m_selectAllAction = new QAction(tr("Select All"), this);
    m_selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(m_selectAllAction, &QAction::triggered, this, &ZTextEditor::selectAll);
    m_contextMenu->addAction(m_selectAllAction);
    
    m_contextMenu->addSeparator();
    
    // Search
    m_searchAction = new QAction(tr("Search"), this);
    m_searchAction->setShortcut(QKeySequence("Ctrl+F"));
    connect(m_searchAction, &QAction::triggered, this, &ZTextEditor::toggleSearchWidget);
    m_contextMenu->addAction(m_searchAction);
}

void ZTextEditor::updateContextMenuActions()
{
    // Update undo/redo status
    m_undoAction->setEnabled(document()->isUndoAvailable());
    m_redoAction->setEnabled(document()->isRedoAvailable());
    
    // Update cut/copy/delete status
    bool hasSelection = textCursor().hasSelection();
    m_cutAction->setEnabled(hasSelection && !isReadOnly());
    m_copyAction->setEnabled(hasSelection);
    m_deleteAction->setEnabled(hasSelection && !isReadOnly());
    
    // Update paste status
    m_pasteAction->setEnabled(!isReadOnly() && QApplication::clipboard()->text().length() > 0);
    
    // Update select all status
    m_selectAllAction->setEnabled(document()->characterCount() > 1); // >1 because includes final null character
}

void ZTextEditor::addContextMenu(QMenu *menu)
{
    if (menu && m_contextMenu) {
        m_contextMenu->addMenu(menu);
    }
}

void ZTextEditor::addContextAction(QAction *action)
{
    if (action && m_contextMenu) {
        m_contextMenu->addAction(action);
    }
}

void ZTextEditor::addContextSeparator()
{
    if (m_contextMenu) {
        m_contextMenu->addSeparator();
    }
}

void ZTextEditor::undo()
{
    QPlainTextEdit::undo();
}

void ZTextEditor::redo()
{
    QPlainTextEdit::redo();
}

void ZTextEditor::cut()
{
    QPlainTextEdit::cut();
}

void ZTextEditor::copy()
{
    QPlainTextEdit::copy();
}

void ZTextEditor::paste()
{
    QPlainTextEdit::paste();
}

void ZTextEditor::selectAll()
{
    QPlainTextEdit::selectAll();
}

void ZTextEditor::deleteSelected()
{
    if (textCursor().hasSelection() && !isReadOnly()) {
        textCursor().removeSelectedText();
    }
}

// Search functionality implementation
void ZTextEditor::setupSearch()
{
    // Create highlighter
    m_highlighter = new ZTextHighlighter(this);
    
    // Create search widget as a child widget
    m_searchWG = new SearchWG(this);
    m_searchWG->setWindowTitle(tr("Search"));
    
    // Set background color for search widget to avoid transparency
    m_searchWG->setAutoFillBackground(true);
    QPalette palette = m_searchWG->palette();
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    m_searchWG->setPalette(palette);
    
    // Configure to show only the required group boxes
    auto requiredBoxes = SearchWG::MatchControl | SearchWG::Operation;
    m_searchWG->setVisibleGroupBoxes(requiredBoxes);
    
    // Initially hide the search widget
    m_searchWG->hide();
    
    // Create Ctrl+F shortcut to toggle search widget
    m_searchShortcut = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(m_searchShortcut, &QShortcut::activated, this, &ZTextEditor::toggleSearchWidget);
    
    // Connect search widget signals
    connect(m_searchWG, &SearchWG::searchReady, this, &ZTextEditor::onSearchReady);
    connect(m_searchWG, &SearchWG::matchControlChanged, this, &ZTextEditor::onSearchReady);
    connect(m_searchWG, &SearchWG::searchClear, this, &ZTextEditor::onSearchClear);
    connect(m_searchWG, &SearchWG::searchNext, this, &ZTextEditor::onSearchNext);
    connect(m_searchWG, &SearchWG::searchBefore, this, &ZTextEditor::onSearchBefore);
    
    // Connect highlighter signals
    connect(m_highlighter, &ZTextHighlighter::highlightCountChanged, this, &ZTextEditor::onHighlightCountChanged);
    connect(m_highlighter, &ZTextHighlighter::currentHighlightChanged, this, &ZTextEditor::onCurrentHighlightChanged);
    connect(m_highlighter, &ZTextHighlighter::searchTextNotFound, this, &ZTextEditor::onSearchTextNotFound);
}

void ZTextEditor::enableSearch(bool enable)
{
    m_searchEnabled = enable;
    if (m_searchWG) {
        if (!enable) {
            m_searchWG->hide();
        }
    }
    if (m_searchAction) {
        m_searchAction->setEnabled(enable);
    }
    updateLineNumberAreaWidth();
}

bool ZTextEditor::isSearchEnabled() const
{
    return m_searchEnabled;
}

SearchWG *ZTextEditor::searchWidget() const
{
    return m_searchWG;
}

ZTextHighlighter *ZTextEditor::highlighter() const
{
    return m_highlighter;
}

void ZTextEditor::toggleSearchWidget()
{
    if (!m_searchEnabled || !m_searchWG) {
        return;
    }
    
    if (m_searchWG->isVisible()) {
        m_searchWG->hide();
        updateLineNumberAreaWidth();
    } else {
        // Position the search widget at the bottom
        int searchHeight = m_searchWG->sizeHint().height();
        m_searchWG->setGeometry(0, height() - searchHeight, width(), searchHeight);
        m_searchWG->show();
        updateLineNumberAreaWidth();
        
        // Focus on the search line edit
        QLineEdit *searchLineEdit = m_searchWG->getSearchLE();
        if (searchLineEdit) {
            searchLineEdit->setFocus();
            searchLineEdit->selectAll();
        }
    }
}

void ZTextEditor::onSearchReady()
{
    QString searchText = m_searchWG->getSearchText().trimmed();
    if (searchText.isEmpty()) {
        m_searchWG->setSearchStatus(tr("Search text is empty"));
        return;
    }
    
    // Check if there's content to search
    if (toPlainText().isEmpty()) {
        m_searchWG->setSearchStatus(tr("No content to search"));
        return;
    }
    
    m_highlighter->setCaseSensitive(m_searchWG->isCaseSensitive());
    m_highlighter->setWholeWord(m_searchWG->isMatchWholewords());
    m_highlighter->setUseRegex(m_searchWG->isUseRegularExpression());
    
    m_highlighter->highlight(searchText);
}

void ZTextEditor::onSearchClear()
{
    m_highlighter->clearHighlight();
    m_searchWG->setSearchText("");
    m_searchWG->setSearchStatus("");
}

void ZTextEditor::onSearchNext()
{
    m_highlighter->gotoNextHighlight();
}

void ZTextEditor::onSearchBefore()
{
    m_highlighter->gotoPreviousHighlight();
}

void ZTextEditor::onHighlightCountChanged(int count)
{
    m_searchWG->setSearchStatus(QString("Found %1 results").arg(count));
}

void ZTextEditor::onCurrentHighlightChanged(int index)
{
    if (index >= 0) {
        m_searchWG->setSearchStatus(QString("Result %1 of %2").arg(index + 1).arg(m_highlighter->highlightCount()));
    }
}

void ZTextEditor::onSearchTextNotFound(const QString &searchText)
{
    m_searchWG->setSearchStatus(QString("Text '%1' not found").arg(searchText));
}

// ─── Syntax Highlighting ────────────────────────────────────────────────────

void ZTextEditor::setupSyntaxHighlighter()
{
    m_syntaxHighlighter = new ZSyntaxHighlighter(document());
    m_syntaxHighlighter->setBaseFont(font());
}

void ZTextEditor::setSyntaxMode(ZHighlightMode mode)
{
    if (m_syntaxHighlighter) {
        m_syntaxHighlighter->setMode(mode);
    }
}

ZHighlightMode ZTextEditor::syntaxMode() const
{
    return m_syntaxHighlighter ? m_syntaxHighlighter->mode() : ZHighlightMode::PlainText;
}

ZSyntaxHighlighter *ZTextEditor::syntaxHighlighter() const
{
    return m_syntaxHighlighter;
}
