// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZTEXTHIGHLIGHTER_H
#define ZTEXTHIGHLIGHTER_H

#include <QObject>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QColor>
#include <QRegularExpression>
#include <QList>

class ZTextHighlighter : public QObject
{
    Q_OBJECT

public:
    explicit ZTextHighlighter(QPlainTextEdit *parent = nullptr);
    ~ZTextHighlighter();

    // Highlight search
    void highlight(const QString &searchText);
    void highlight(const QString &searchText, const QColor &backgroundColor, const QColor &textColor = Qt::black);

    // Clear highlights
    void clearHighlight();

    // Navigation functions
    void gotoNextHighlight();
    void gotoPreviousHighlight();
    void gotoFirstHighlight();
    void gotoLastHighlight();

    // Configuration options
    void setCaseSensitive(bool caseSensitive);
    void setWholeWord(bool wholeWord);
    void setUseRegex(bool useRegex);
    void setHighlightColor(const QColor &backgroundColor, const QColor &textColor = Qt::black);

    // Status getters
    int highlightCount() const;
    int currentHighlightIndex() const;
    bool hasHighlights() const;
    QString currentSearchText() const;

    // Highlight all matches and return count
    int highlightAll(const QString &searchText);

signals:
    void highlightCountChanged(int count);
    void currentHighlightChanged(int index);
    void searchTextNotFound(const QString &searchText);

private:
    void applyHighlightFormat(QTextCursor &cursor);
    bool isValidSearchText(const QString &text) const;
    QTextDocument::FindFlags getFindFlags() const;

private:
    QPlainTextEdit *m_textEdit = nullptr;
    QList<QTextCursor> m_highlightedCursors;
    QTextCharFormat m_highlightFormat;
    QString m_currentSearchText;
    bool m_caseSensitive;
    bool m_wholeWord;
    bool m_useRegex;
    int m_currentIndex;
};

#endif // ZTEXTHIGHLIGHTER_H
