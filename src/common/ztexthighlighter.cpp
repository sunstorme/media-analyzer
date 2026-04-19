// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "ztexthighlighter.h"
#include "zhighlightconfig.h"
#include <QDebug>

ZTextHighlighter::ZTextHighlighter(QPlainTextEdit *parent)
    : QObject(parent)
    , m_textEdit(parent)
    , m_caseSensitive(false)
    , m_wholeWord(false)
    , m_useRegex(false)
    , m_currentIndex(-1)
{
    // Set default highlight format from centralized config
    m_highlightFormat = ZHighlightConfig::searchHighlightFormat();
}

ZTextHighlighter::~ZTextHighlighter()
{
    // clearHighlight();
}

void ZTextHighlighter::highlight(const QString &searchText)
{
    highlight(searchText, m_highlightFormat.background().color(), m_highlightFormat.foreground().color());
}

void ZTextHighlighter::highlight(const QString &searchText, const QColor &backgroundColor, const QColor &textColor)
{
    if (!m_textEdit || !isValidSearchText(searchText)) {
        return;
    }

    // Save current search text
    m_currentSearchText = searchText;

    // Clear old highlights
    clearHighlight();

    // Set highlight format
    m_highlightFormat.setBackground(backgroundColor);
    m_highlightFormat.setForeground(textColor);

    QTextCursor originalCursor = m_textEdit->textCursor();
    QTextCursor cursor(m_textEdit->document());
    cursor.movePosition(QTextCursor::Start);

    m_textEdit->setTextCursor(cursor);

    int foundCount = 0;
    bool found = false;

    if (m_useRegex) {
        // Regex search
        QRegularExpression regex(searchText, m_caseSensitive ? QRegularExpression::NoPatternOption
                                                             : QRegularExpression::CaseInsensitiveOption);
        if (!regex.isValid()) {
            qWarning() << "Invalid regex pattern:" << regex.errorString();
            emit searchTextNotFound(searchText);
            return;
        }

        QRegularExpressionMatchIterator it = regex.globalMatch(m_textEdit->toPlainText());
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QTextCursor foundCursor(m_textEdit->document());
            foundCursor.setPosition(match.capturedStart());
            foundCursor.setPosition(match.capturedEnd(), QTextCursor::KeepAnchor);

            foundCursor.mergeCharFormat(m_highlightFormat);
            m_highlightedCursors.append(foundCursor);
            foundCount++;
        }
    } else {
        // Plain text search
        QTextDocument::FindFlags flags = getFindFlags();

        while (m_textEdit->find(searchText, flags)) {
            QTextCursor foundCursor = m_textEdit->textCursor();
            applyHighlightFormat(foundCursor);
            m_highlightedCursors.append(foundCursor);
            foundCount++;
            found = true;
        }
    }

    // Restore original cursor position
    m_textEdit->setTextCursor(originalCursor);

    if (foundCount > 0) {
        m_currentIndex = 0;
        emit highlightCountChanged(foundCount);
        emit currentHighlightChanged(0);
    } else {
        m_currentIndex = -1;
        emit searchTextNotFound(searchText);
    }
}

void ZTextHighlighter::clearHighlight()
{
    if (!m_textEdit) {
        return;
    }

    // Clear all stored highlight cursors
    QTextCharFormat plainFormat;
    for (QTextCursor &cursor : m_highlightedCursors) {
        cursor.mergeCharFormat(plainFormat);
    }

    // Reset entire editor format to ensure no highlight remnants
    QTextCursor cursor(m_textEdit->document());
    cursor.select(QTextCursor::Document);
    cursor.setCharFormat(plainFormat);
    cursor.clearSelection();

    m_highlightedCursors.clear();
    m_currentIndex = -1;
    m_currentSearchText.clear();

    emit highlightCountChanged(0);
    emit currentHighlightChanged(-1);
}

void ZTextHighlighter::gotoNextHighlight()
{
    if (m_highlightedCursors.isEmpty()) {
        return;
    }

    m_currentIndex = (m_currentIndex + 1) % m_highlightedCursors.size();
    m_textEdit->setTextCursor(m_highlightedCursors[m_currentIndex]);
    m_textEdit->ensureCursorVisible();

    emit currentHighlightChanged(m_currentIndex);
}

void ZTextHighlighter::gotoPreviousHighlight()
{
    if (m_highlightedCursors.isEmpty()) {
        return;
    }

    m_currentIndex = (m_currentIndex - 1 + m_highlightedCursors.size()) % m_highlightedCursors.size();
    m_textEdit->setTextCursor(m_highlightedCursors[m_currentIndex]);
    m_textEdit->ensureCursorVisible();

    emit currentHighlightChanged(m_currentIndex);
}

void ZTextHighlighter::gotoFirstHighlight()
{
    if (m_highlightedCursors.isEmpty()) {
        return;
    }

    m_currentIndex = 0;
    m_textEdit->setTextCursor(m_highlightedCursors.first());
    m_textEdit->ensureCursorVisible();

    emit currentHighlightChanged(0);
}

void ZTextHighlighter::gotoLastHighlight()
{
    if (m_highlightedCursors.isEmpty()) {
        return;
    }

    m_currentIndex = m_highlightedCursors.size() - 1;
    m_textEdit->setTextCursor(m_highlightedCursors.last());
    m_textEdit->ensureCursorVisible();

    emit currentHighlightChanged(m_currentIndex);
}

void ZTextHighlighter::setCaseSensitive(bool caseSensitive)
{
    m_caseSensitive = caseSensitive;
}

void ZTextHighlighter::setWholeWord(bool wholeWord)
{
    m_wholeWord = wholeWord;
}

void ZTextHighlighter::setUseRegex(bool useRegex)
{
    m_useRegex = useRegex;
}

void ZTextHighlighter::setHighlightColor(const QColor &backgroundColor, const QColor &textColor)
{
    m_highlightFormat.setBackground(backgroundColor);
    m_highlightFormat.setForeground(textColor);
}

int ZTextHighlighter::highlightCount() const
{
    return m_highlightedCursors.size();
}

int ZTextHighlighter::currentHighlightIndex() const
{
    return m_currentIndex;
}

bool ZTextHighlighter::hasHighlights() const
{
    return !m_highlightedCursors.isEmpty();
}

QString ZTextHighlighter::currentSearchText() const
{
    return m_currentSearchText;
}

int ZTextHighlighter::highlightAll(const QString &searchText)
{
    highlight(searchText);
    return highlightCount();
}

void ZTextHighlighter::applyHighlightFormat(QTextCursor &cursor)
{
    cursor.mergeCharFormat(m_highlightFormat);
}

bool ZTextHighlighter::isValidSearchText(const QString &text) const
{
    return !text.trimmed().isEmpty();
}

QTextDocument::FindFlags ZTextHighlighter::getFindFlags() const
{
    QTextDocument::FindFlags flags;
    if (m_caseSensitive) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (m_wholeWord) {
        flags |= QTextDocument::FindWholeWords;
    }
    return flags;
}
