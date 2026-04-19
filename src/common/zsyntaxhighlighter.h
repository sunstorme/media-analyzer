// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZSYNTAXHIGHLIGHTER_H
#define ZSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QFont>

/**
 * @brief Syntax highlighting modes supported by the highlighter
 */
enum class ZHighlightMode {
    PlainText,  ///< No syntax highlighting
    Json,       ///< JSON syntax highlighting
    Log,        ///< Log level highlighting
    Table       ///< Table/CSV data highlighting
};

/**
 * @brief QSyntaxHighlighter subclass providing JSON and log level highlighting
 *
 * Uses QSyntaxHighlighter for efficient, automatic re-highlighting on text changes.
 * Color and style definitions are sourced exclusively from ZHighlightConfig
 * to maintain separation of concerns between style and logic.
 *
 * Usage:
 * @code
 * ZSyntaxHighlighter *hl = new ZSyntaxHighlighter(textDocument);
 * hl->setMode(ZHighlightMode::Json);
 * @endcode
 */
class ZSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit ZSyntaxHighlighter(QTextDocument *parent = nullptr);
    ~ZSyntaxHighlighter() override;

    /**
     * @brief Set the highlighting mode
     * @param mode The syntax mode to use (PlainText, Json, or Log)
     */
    void setMode(ZHighlightMode mode);

    /**
     * @brief Get the current highlighting mode
     * @return Current mode
     */
    ZHighlightMode mode() const;

    /**
     * @brief Set the base font used for format derivation
     * @param font The base font
     */
    void setBaseFont(const QFont &font);

    /**
     * @brief Get the current base font
     * @return Current base font
     */
    QFont baseFont() const;

protected:
    void highlightBlock(const QString &text) override;

private:
    /// Highlight a block as JSON
    void highlightJsonBlock(const QString &text);

    /// Highlight a block as a log line
    void highlightLogBlock(const QString &text);

    /// Highlight a block as table/CSV data
    void highlightTableBlock(const QString &text);

    /// Apply a regex match with the given format
    void applyFormat(const QString &text, const QRegularExpression &regex,
                     const QTextCharFormat &format, int captureGroup = 0);

    /// Cached JSON highlight rules (rebuilt when font changes)
    struct JsonRule {
        QRegularExpression pattern;
        QTextCharFormat format;
        int captureGroup;
    };

    /// Build the JSON rule set from ZHighlightConfig
    void buildJsonRules();

    /// Build the log rule set from ZHighlightConfig
    void buildLogRules();

    /// Build the table rule set from ZHighlightConfig
    void buildTableRules();

    ZHighlightMode m_mode = ZHighlightMode::PlainText;
    QFont m_baseFont;

    QList<JsonRule> m_jsonRules;
    QList<JsonRule> m_logRules;
    QList<JsonRule> m_tableRules;

    // Log-specific regex patterns (precompiled for performance)
    QRegularExpression m_logTimestampRegex;
    QRegularExpression m_logLevelRegex;
    QRegularExpression m_logBracketRegex;
};

#endif // ZSYNTAXHIGHLIGHTER_H
