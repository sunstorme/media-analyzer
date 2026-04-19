// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZTEXTEDITOR_H
#define ZTEXTEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QShortcut>

#include "zsyntaxhighlighter.h"

class ZLineNumberArea;
class ZTextHighlighter;
class SearchWG;

class ZTextEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    ZTextEditor(QWidget *parent = nullptr);

    int lineNumberAreaWidth();
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    
    // Context menu interface
    void addContextMenu(QMenu *menu);
    void addContextAction(QAction *action);
    void addContextSeparator();
    
    // Search functionality
    void enableSearch(bool enable = true);
    bool isSearchEnabled() const;
    SearchWG *searchWidget() const;
    ZTextHighlighter *highlighter() const;

    // Syntax highlighting
    void setSyntaxMode(ZHighlightMode mode);
    ZHighlightMode syntaxMode() const;
    ZSyntaxHighlighter *syntaxHighlighter() const;

private slots:
    void updateLineNumberAreaWidth();
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();
    
    // Context menu slot functions
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    void deleteSelected();
    
    // Search slot functions
    void onSearchReady();
    void onSearchClear();
    void onSearchNext();
    void onSearchBefore();
    void onHighlightCountChanged(int count);
    void onCurrentHighlightChanged(int index);
    void onSearchTextNotFound(const QString &searchText);
    void toggleSearchWidget();

private:
    ZLineNumberArea *m_lineNumberArea = nullptr;
    
    // Context menu related
    QMenu *m_contextMenu = nullptr;
    QAction *m_undoAction = nullptr;
    QAction *m_redoAction = nullptr;
    QAction *m_cutAction = nullptr;
    QAction *m_copyAction = nullptr;
    QAction *m_pasteAction = nullptr;
    QAction *m_selectAllAction = nullptr;
    QAction *m_deleteAction = nullptr;
    QAction *m_searchAction = nullptr;
    
    // Search related
    ZTextHighlighter *m_highlighter = nullptr;
    SearchWG *m_searchWG = nullptr;
    QShortcut *m_searchShortcut = nullptr;
    bool m_searchEnabled = true;
    
    // Syntax highlighting
    ZSyntaxHighlighter *m_syntaxHighlighter = nullptr;
    
    void setupContextMenu();
    void updateContextMenuActions();
    void setupSearch();
    void setupSyntaxHighlighter();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
};

class ZLineNumberArea : public QWidget
{
public:
    ZLineNumberArea(ZTextEditor *editor) : QWidget(editor), m_editor(editor) {}

    QSize sizeHint() const override {
        return QSize(m_editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        m_editor->lineNumberAreaPaintEvent(event);
    }

private:
    ZTextEditor *m_editor;
};

#endif // ZTEXTEDITOR_H
    
