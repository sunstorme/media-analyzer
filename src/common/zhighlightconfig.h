// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZHIGHLIGHTCONFIG_H
#define ZHIGHLIGHTCONFIG_H

#include <QColor>
#include <QTextCharFormat>
#include <QFont>

/**
 * @brief Global highlight color and style configuration class
 *
 * Centralizes all syntax highlighting color definitions and text style
 * configurations used across JSON syntax highlighting, log level highlighting,
 * search highlighting, and editor chrome (line numbers, current line, etc.).
 *
 * Color palette design principles:
 * - All syntax colors are chosen to avoid visual conflict with the search
 *   highlight background (yellow #FFFF64) and foreground (black #000000).
 * - JSON token colors follow GitHub/VS Code conventions for familiarity.
 * - Log level colors use intuitive semantic mapping (red=error, orange=warn, etc.).
 * - Colors maintain WCAG AA contrast ratio against white (#FFFFFF) background.
 */
class ZHighlightConfig
{
public:
    // ─── Editor Chrome Colors ───────────────────────────────────────────
    struct Editor {
        static QColor lineNumberBackground()   { return QColor(247, 247, 247); }
        static QColor lineNumberText()          { return QColor(153, 153, 153); }
        static QColor currentLineBackground()   { return QColor(247, 247, 247); }
        static QColor currentLineNumber()       { return QColor(55, 118, 171);  }
        static QColor editorBackground()        { return QColor(255, 255, 255); }
        static constexpr int lineNumberRightMargin() { return 8; }
    };

    // ─── Search Highlight Colors ────────────────────────────────────────
    struct Search {
        static QColor highlightBackground() { return QColor(255, 255, 100); }
        static QColor highlightForeground() { return QColor(0, 0, 0);        }
    };

    // ─── JSON Syntax Highlight Colors ───────────────────────────────────
    // Palette: Inspired by GitHub light theme with adjustments for clarity.
    // None of these colors use yellow backgrounds, ensuring no conflict
    // with the search highlight (yellow #FFFF64 bg, black fg).
    struct Json {
        /** JSON object keys (e.g. "name") — deep blue */
        static QColor key()              { return QColor(4, 81, 165);    }   // #0451A5
        /** JSON string values (e.g. "hello") — dark teal */
        static QColor string()           { return QColor(3, 47, 98);     }   // #032F62
        /** JSON numeric values (e.g. 42, 3.14) — vivid blue */
        static QColor number()           { return QColor(0, 92, 197);    }   // #005CC5
        /** JSON boolean literals (true/false) — burnt orange */
        static QColor boolean()          { return QColor(227, 98, 9);    }   // #E36209
        /** JSON null literal — medium purple */
        static QColor null()             { return QColor(111, 66, 193);  }   // #6F42C1
        /** Structural braces and brackets { } [ ] — dark slate */
        static QColor brace()            { return QColor(36, 41, 46);    }   // #24292E
        /** Colon and comma punctuation : , — dark slate */
        static QColor punctuation()      { return QColor(36, 41, 46);    }   // #24292E
    };

    // ─── Log Level Highlight Colors ─────────────────────────────────────
    // Palette: Semantic color mapping with consistent foreground-only styling.
    // All levels use foreground coloring only (no background), ensuring
    // zero conflict with search highlight's yellow background.
    struct Log {
        // ── Level foreground colors ──
        /** DEBUG level — muted gray */
        static QColor debugForeground()     { return QColor(106, 115, 125); }  // #6A737D
        /** INFO level — calm blue */
        static QColor infoForeground()      { return QColor(0, 92, 197);     }  // #005CC5
        /** WARNING level — amber orange */
        static QColor warningForeground()   { return QColor(227, 98, 9);     }  // #E36209
        /** ERROR level — vivid red */
        static QColor errorForeground()     { return QColor(215, 58, 73);    }  // #D73A49
        /** FATAL level — deep crimson, bold */
        static QColor fatalForeground()     { return QColor(179, 29, 56);    }  // #B31D38

        // ── Level background colors (subtle, for optional use) ──
        /** DEBUG level — very light gray */
        static QColor debugBackground()     { return QColor(245, 245, 245); }  // #F5F5F5
        /** INFO level — very light blue */
        static QColor infoBackground()      { return QColor(236, 244, 255); }  // #ECF4FF
        /** WARNING level — very light amber */
        static QColor warningBackground()   { return QColor(255, 244, 231); }  // #FFF4E7
        /** ERROR level — very light red */
        static QColor errorBackground()     { return QColor(255, 237, 237); }  // #FFEDED
        /** FATAL level — light pink */
        static QColor fatalBackground()     { return QColor(255, 228, 230); }  // #FFE4E6

        // ── Timestamp styling ──
        /** Timestamp text — steel blue */
        static QColor timestampForeground() { return QColor(47, 90, 135);    }  // #2F5A87

        // ── Bracket/tag styling ──
        /** Bracket/tag text [ ] — medium gray */
        static QColor bracketForeground()   { return QColor(128, 128, 128);  }  // #808080

        // ── Level keyword strings (for regex matching) ──
        static QString debugKeyword()   { return QStringLiteral("DEBUG");    }
        static QString infoKeyword()    { return QStringLiteral("INFO");     }
        static QString warningKeyword() { return QStringLiteral("WARNING");  }
        static QString errorKeyword()   { return QStringLiteral("ERROR");    }
        static QString fatalKeyword()   { return QStringLiteral("FATAL");    }
    };

    // ─── Table/CSV Highlight Colors ─────────────────────────────────────
    // Palette: Designed for tabular data with clear header/data distinction.
    struct Table {
        /** Header row (first line) — dark teal, bold */
        static QColor headerForeground()    { return QColor(0, 80, 110);     }  // #00506E
        /** Separator characters (tab, comma) — medium gray */
        static QColor separatorForeground() { return QColor(180, 180, 180);  }  // #B4B4B4
        /** Numeric cell values — vivid blue */
        static QColor numberForeground()    { return QColor(0, 92, 197);     }  // #005CC5
        /** Quoted string values — dark teal */
        static QColor quotedForeground()    { return QColor(3, 47, 98);      }  // #032F62
        /** Key-value separator (=) — burnt orange */
        static QColor kvSeparatorForeground() { return QColor(227, 98, 9);   }  // #E36209
    };

    // ─── Format Builders ────────────────────────────────────────────────
    // Pre-built QTextCharFormat instances for common use cases.
    // These are lightweight to construct and can be used directly.

    /**
     * @brief Create a QTextCharFormat for JSON keys
     * @param baseFont The base font to apply styling on top of
     * @return Configured format with key color and optional bold
     */
    static QTextCharFormat jsonKeyFormat(const QFont &baseFont)
    {
        QTextCharFormat fmt;
        fmt.setForeground(Json::key());
        QFont f(baseFont);
        f.setBold(true);
        fmt.setFont(f);
        return fmt;
    }

    /**
     * @brief Create a QTextCharFormat for JSON string values
     */
    static QTextCharFormat jsonStringFormat(const QFont &baseFont)
    {
        Q_UNUSED(baseFont)
        QTextCharFormat fmt;
        fmt.setForeground(Json::string());
        return fmt;
    }

    /**
     * @brief Create a QTextCharFormat for JSON numeric values
     */
    static QTextCharFormat jsonNumberFormat(const QFont &baseFont)
    {
        Q_UNUSED(baseFont)
        QTextCharFormat fmt;
        fmt.setForeground(Json::number());
        return fmt;
    }

    /**
     * @brief Create a QTextCharFormat for JSON boolean literals
     */
    static QTextCharFormat jsonBooleanFormat(const QFont &baseFont)
    {
        Q_UNUSED(baseFont)
        QTextCharFormat fmt;
        fmt.setForeground(Json::boolean());
        return fmt;
    }

    /**
     * @brief Create a QTextCharFormat for JSON null literal
     */
    static QTextCharFormat jsonNullFormat(const QFont &baseFont)
    {
        Q_UNUSED(baseFont)
        QTextCharFormat fmt;
        fmt.setForeground(Json::null());
        return fmt;
    }

    /**
     * @brief Create a QTextCharFormat for JSON structural braces
     */
    static QTextCharFormat jsonBraceFormat(const QFont &baseFont)
    {
        Q_UNUSED(baseFont)
        QTextCharFormat fmt;
        fmt.setForeground(Json::brace());
        return fmt;
    }

    /**
     * @brief Create a QTextCharFormat for a log level by name
     * @param level The log level string (DEBUG, INFO, WARNING, ERROR, FATAL)
     * @param baseFont The base font to apply styling on top of
     * @return Configured format with appropriate foreground color
     */
    static QTextCharFormat logLevelFormat(const QString &level, const QFont &baseFont)
    {
        QTextCharFormat fmt;
        fmt.setFontWeight(QFont::Bold);

        QString upper = level.toUpper();
        if (upper == Log::debugKeyword()) {
            fmt.setForeground(Log::debugForeground());
            fmt.setFontWeight(QFont::Normal);
        } else if (upper == Log::infoKeyword()) {
            fmt.setForeground(Log::infoForeground());
        } else if (upper == Log::warningKeyword()) {
            fmt.setForeground(Log::warningForeground());
        } else if (upper == Log::errorKeyword()) {
            fmt.setForeground(Log::errorForeground());
        } else if (upper == Log::fatalKeyword()) {
            fmt.setForeground(Log::fatalForeground());
        } else {
            fmt.setForeground(QColor(0, 0, 0));
            fmt.setFontWeight(QFont::Normal);
        }

        Q_UNUSED(baseFont)
        return fmt;
    }

    /**
     * @brief Create a QTextCharFormat for log timestamps
     */
    static QTextCharFormat logTimestampFormat(const QFont &baseFont)
    {
        Q_UNUSED(baseFont)
        QTextCharFormat fmt;
        fmt.setForeground(Log::timestampForeground());
        return fmt;
    }

    /**
     * @brief Create a QTextCharFormat for log brackets/tags
     */
    static QTextCharFormat logBracketFormat(const QFont &baseFont)
    {
        Q_UNUSED(baseFont)
        QTextCharFormat fmt;
        fmt.setForeground(Log::bracketForeground());
        return fmt;
    }

    /**
     * @brief Create the search highlight format
     */
    static QTextCharFormat searchHighlightFormat()
    {
        QTextCharFormat fmt;
        fmt.setBackground(Search::highlightBackground());
        fmt.setForeground(Search::highlightForeground());
        return fmt;
    }

    /**
     * @brief Create a QTextCharFormat for table header row
     */
    static QTextCharFormat tableHeaderFormat(const QFont &baseFont)
    {
        QTextCharFormat fmt;
        fmt.setForeground(Table::headerForeground());
        QFont f(baseFont);
        f.setBold(true);
        fmt.setFont(f);
        return fmt;
    }

    /**
     * @brief Create a QTextCharFormat for table separators
     */
    static QTextCharFormat tableSeparatorFormat(const QFont &baseFont)
    {
        Q_UNUSED(baseFont)
        QTextCharFormat fmt;
        fmt.setForeground(Table::separatorForeground());
        return fmt;
    }

    /**
     * @brief Create a QTextCharFormat for table numeric values
     */
    static QTextCharFormat tableNumberFormat(const QFont &baseFont)
    {
        Q_UNUSED(baseFont)
        QTextCharFormat fmt;
        fmt.setForeground(Table::numberForeground());
        return fmt;
    }

    /**
     * @brief Create a QTextCharFormat for table key-value separators
     */
    static QTextCharFormat tableKVSeparatorFormat(const QFont &baseFont)
    {
        Q_UNUSED(baseFont)
        QTextCharFormat fmt;
        fmt.setForeground(Table::kvSeparatorForeground());
        fmt.setFontWeight(QFont::Bold);
        return fmt;
    }
};

#endif // ZHIGHLIGHTCONFIG_H
