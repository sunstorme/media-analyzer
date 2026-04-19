// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zsyntaxhighlighter.h"
#include "zhighlightconfig.h"

#include <QDebug>

// ────────────────────────────────────────────────────────────────────────────
// Construction / Destruction
// ────────────────────────────────────────────────────────────────────────────

ZSyntaxHighlighter::ZSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    buildJsonRules();
    buildLogRules();
    buildTableRules();
}

ZSyntaxHighlighter::~ZSyntaxHighlighter() = default;

// ────────────────────────────────────────────────────────────────────────────
// Public Interface
// ────────────────────────────────────────────────────────────────────────────

void ZSyntaxHighlighter::setMode(ZHighlightMode mode)
{
    if (m_mode == mode)
        return;
    m_mode = mode;
    rehighlight();
}

ZHighlightMode ZSyntaxHighlighter::mode() const
{
    return m_mode;
}

void ZSyntaxHighlighter::setBaseFont(const QFont &font)
{
    if (m_baseFont == font)
        return;
    m_baseFont = font;
    // Rebuild rules that depend on font
    buildJsonRules();
    buildLogRules();
    buildTableRules();
    rehighlight();
}

QFont ZSyntaxHighlighter::baseFont() const
{
    return m_baseFont;
}

// ────────────────────────────────────────────────────────────────────────────
// QSyntaxHighlighter Interface
// ────────────────────────────────────────────────────────────────────────────

void ZSyntaxHighlighter::highlightBlock(const QString &text)
{
    switch (m_mode) {
    case ZHighlightMode::Json:
        highlightJsonBlock(text);
        break;
    case ZHighlightMode::Log:
        highlightLogBlock(text);
        break;
    case ZHighlightMode::Table:
        highlightTableBlock(text);
        break;
    case ZHighlightMode::PlainText:
    default:
        // No highlighting for plain text
        break;
    }
}

// ────────────────────────────────────────────────────────────────────────────
// JSON Highlighting
// ────────────────────────────────────────────────────────────────────────────

void ZSyntaxHighlighter::highlightJsonBlock(const QString &text)
{
    // Apply pre-built rules in order. Rules are ordered so that more specific
    // patterns take priority. Later rules can overwrite earlier ones within
    // the same character range, which is the desired behaviour for nested
    // JSON constructs (e.g. a key inside braces).
    for (const auto &rule : m_jsonRules) {
        applyFormat(text, rule.pattern, rule.format, rule.captureGroup);
    }
}

void ZSyntaxHighlighter::buildJsonRules()
{
    m_jsonRules.clear();

    // ── Rule ordering matters ────────────────────────────────────────────
    // 1. Strings (double-quoted) — must be matched first to prevent inner
    //    content from being highlighted by later rules.
    // 2. Key detection — a string followed by a colon is a key, not a value.
    // 3. Primitive literals: true, false, null.
    // 4. Numbers — integer and floating-point.
    // 5. Structural punctuation: { } [ ] : ,

    // Rule 1: String values (any double-quoted content)
    // This is the broadest string rule; it will be partially overridden by
    // the key rule below for strings that precede a colon.
    {
        JsonRule rule;
        // Match a complete JSON string including escaped characters
        rule.pattern = QRegularExpression(
            R"("([^"\\]|\\.)*")",
            QRegularExpression::NoPatternOption
        );
        rule.format = ZHighlightConfig::jsonStringFormat(m_baseFont);
        rule.captureGroup = 0;
        m_jsonRules.append(rule);
    }

    // Rule 2: JSON keys — a double-quoted string immediately followed by ':'
    // This overrides the string format for keys with bold + key color.
    {
        JsonRule rule;
        rule.pattern = QRegularExpression(
            R"("([^"\\]|\\.)*"\s*:)",
            QRegularExpression::NoPatternOption
        );
        rule.format = ZHighlightConfig::jsonKeyFormat(m_baseFont);
        rule.captureGroup = 0;
        m_jsonRules.append(rule);
    }

    // Rule 3: Boolean literals (true / false)
    {
        JsonRule rule;
        rule.pattern = QRegularExpression(
            R"(\b(true|false)\b)",
            QRegularExpression::NoPatternOption
        );
        rule.format = ZHighlightConfig::jsonBooleanFormat(m_baseFont);
        rule.captureGroup = 0;
        m_jsonRules.append(rule);
    }

    // Rule 4: Null literal
    {
        JsonRule rule;
        rule.pattern = QRegularExpression(
            R"(\bnull\b)",
            QRegularExpression::NoPatternOption
        );
        rule.format = ZHighlightConfig::jsonNullFormat(m_baseFont);
        rule.captureGroup = 0;
        m_jsonRules.append(rule);
    }

    // Rule 5: Numeric values (integer and floating-point, including negative)
    {
        JsonRule rule;
        rule.pattern = QRegularExpression(
            R"(\b-?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?\b)",
            QRegularExpression::NoPatternOption
        );
        rule.format = ZHighlightConfig::jsonNumberFormat(m_baseFont);
        rule.captureGroup = 0;
        m_jsonRules.append(rule);
    }
}

// ────────────────────────────────────────────────────────────────────────────
// Log Highlighting
// ────────────────────────────────────────────────────────────────────────────

void ZSyntaxHighlighter::highlightLogBlock(const QString &text)
{
    // Apply pre-built log rules
    for (const auto &rule : m_logRules) {
        applyFormat(text, rule.pattern, rule.format, rule.captureGroup);
    }
}

void ZSyntaxHighlighter::buildLogRules()
{
    m_logRules.clear();

    // ── Log format patterns ──────────────────────────────────────────────
    // Full format:  [timestamp] [LEVEL] [module] message [File] x [Line] n [Fun] func
    // Simple format: [timestamp] [LEVEL] [file:line] message
    //
    // We highlight by priority:
    // 1. Timestamps inside brackets: [yyyy-MM-dd hh:mm:ss.zzz]
    // 2. Log level keywords inside brackets: [INFO], [WARNING], etc.
    // 3. Structural brackets: [ and ]
    // 4. Tags like [File], [Line], [Fun]

    // Rule 1: Timestamp — matches [yyyy-MM-dd hh:mm:ss.zzz] or similar patterns
    {
        JsonRule rule;
        rule.pattern = QRegularExpression(
            R"(\[(\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}(?:\.\d+)?)\])",
            QRegularExpression::NoPatternOption
        );
        rule.format = ZHighlightConfig::logTimestampFormat(m_baseFont);
        // Highlight the entire bracketed timestamp including brackets
        rule.captureGroup = 0;
        m_logRules.append(rule);
    }

    // Rule 2: Log level keywords — [DEBUG], [INFO], [WARNING], [ERROR], [FATAL]
    // We create individual rules per level so each gets its own color.
    QStringList levelKeywords = {
        ZHighlightConfig::Log::debugKeyword(),
        ZHighlightConfig::Log::infoKeyword(),
        ZHighlightConfig::Log::warningKeyword(),
        ZHighlightConfig::Log::errorKeyword(),
        ZHighlightConfig::Log::fatalKeyword(),
    };

    for (const QString &keyword : levelKeywords) {
        JsonRule rule;
        // Match [LEVEL] with optional whitespace
        rule.pattern = QRegularExpression(
            QStringLiteral(R"(\[(\s*%1\s*)\])").arg(keyword),
            QRegularExpression::CaseInsensitiveOption
        );
        rule.format = ZHighlightConfig::logLevelFormat(keyword, m_baseFont);
        rule.captureGroup = 0;  // Highlight the entire [LEVEL] including brackets
        m_logRules.append(rule);
    }

    // Rule 3: Structural tags — [File], [Line], [Fun]
    {
        JsonRule rule;
        rule.pattern = QRegularExpression(
            R"(\[(File|Line|Fun)\])",
            QRegularExpression::NoPatternOption
        );
        rule.format = ZHighlightConfig::logBracketFormat(m_baseFont);
        rule.format.setFontWeight(QFont::Bold);
        rule.captureGroup = 0;
        m_logRules.append(rule);
    }

    // Rule 4: Square brackets (generic, lower priority)
    {
        JsonRule rule;
        rule.pattern = QRegularExpression(
            R"([\[\]])",
            QRegularExpression::NoPatternOption
        );
        rule.format = ZHighlightConfig::logBracketFormat(m_baseFont);
        rule.captureGroup = 0;
        m_logRules.append(rule);
    }
}

// ────────────────────────────────────────────────────────────────────────────
// Utility
// ────────────────────────────────────────────────────────────────────────────

void ZSyntaxHighlighter::applyFormat(const QString &text,
                                      const QRegularExpression &regex,
                                      const QTextCharFormat &format,
                                      int captureGroup)
{
    QRegularExpressionMatchIterator it = regex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart(captureGroup);
        int length = match.capturedLength(captureGroup);
        if (start >= 0 && length > 0) {
            setFormat(start, length, format);
        }
    }
}

// ────────────────────────────────────────────────────────────────────────────
// Table Highlighting
// ────────────────────────────────────────────────────────────────────────────

void ZSyntaxHighlighter::highlightTableBlock(const QString &text)
{
    // The first block (block number 0) is the header row
    bool isHeader = (currentBlock().blockNumber() == 0);

    if (isHeader) {
        // Highlight entire header row with header format
        if (!text.isEmpty()) {
            setFormat(0, text.length(), ZHighlightConfig::tableHeaderFormat(m_baseFont));
        }
        // Still highlight separators within the header for visual clarity
        applyFormat(text, QRegularExpression(R"([\t,])"),
                    ZHighlightConfig::tableSeparatorFormat(m_baseFont));
    } else {
        // Data rows: highlight separators, numbers, and key-value separators
        for (const auto &rule : m_tableRules) {
            applyFormat(text, rule.pattern, rule.format, rule.captureGroup);
        }
    }
}

void ZSyntaxHighlighter::buildTableRules()
{
    m_tableRules.clear();

    // Rule 1: Separator characters (tab and comma)
    {
        JsonRule rule;
        rule.pattern = QRegularExpression(
            R"([\t,])",
            QRegularExpression::NoPatternOption
        );
        rule.format = ZHighlightConfig::tableSeparatorFormat(m_baseFont);
        rule.captureGroup = 0;
        m_tableRules.append(rule);
    }

    // Rule 2: Key-value separator (=)
    {
        JsonRule rule;
        rule.pattern = QRegularExpression(
            R"(=)",
            QRegularExpression::NoPatternOption
        );
        rule.format = ZHighlightConfig::tableKVSeparatorFormat(m_baseFont);
        rule.captureGroup = 0;
        m_tableRules.append(rule);
    }

    // Rule 3: Numeric values (integer and floating-point, including negative)
    {
        JsonRule rule;
        rule.pattern = QRegularExpression(
            R"(\b-?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?\b)",
            QRegularExpression::NoPatternOption
        );
        rule.format = ZHighlightConfig::tableNumberFormat(m_baseFont);
        rule.captureGroup = 0;
        m_tableRules.append(rule);
    }
}
