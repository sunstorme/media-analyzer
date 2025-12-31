// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZLOGGER_H
#define ZLOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QDir>
#include <QtGlobal>
#include <QSettings>

#include "common/common.h"

// Macros for convenient usage
#define LOG_DEBUG(module, message)    ZLogger::instance()->debug(module, message)
#define LOG_INFO(module, message)     ZLogger::instance()->info(module, message)
#define LOG_WARNING(module, message)  ZLogger::instance()->warning(module, message)
#define LOG_ERROR(module, message)    ZLogger::instance()->error(module, message)
#define LOG_FATAL(module, message)    ZLogger::instance()->fatal(module, message)
/**
 * @brief Log level enumeration
 */
enum LogLevel {
    LOG_DEBUG,   // Debug messages
    LOG_INFO,    // Informational messages
    LOG_WARNING, // Warning messages
    LOG_ERROR,   // Error messages
    LOG_FATAL    // Critical error messages
};

/**
 * @brief High-performance logging utility class
 * @details Supports multi-level logging, file rotation, thread safety, and Qt message handler integration
 */
class ZLogger : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Get logger instance (singleton pattern)
     * @return Logger instance pointer
     */
    static ZLogger* instance();

    /**
     * @brief Initialize the logging system
     * @param logDir Log directory
     * @param maxSize Maximum size of a single log file (MB)
     * @param maxFiles Maximum number of files to keep
     * @param installMessageHandler Whether to install Qt message handler
     * @return Whether initialization succeeded
     */
    bool initialize(const QString& logDir,
                    quint64 maxSize = 10,
                    int maxFiles = 7,
                    bool installMessageHandler = true);

    /**
     * @brief Write log entry
     * @param level Log level
     * @param module Module name
     * @param message Log message
     */
    void write(LogLevel level, const QString& module, const QString& message);

    /**
     * @brief Handle Qt system messages
     * @param type Qt message type
     * @param context Message context
     * @param msg Message content
     */
    static void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    /**
     * @brief Shortcut method: write DEBUG level log
     */
    void debug(const QString& module, const QString& message);

    /**
     * @brief Shortcut method: write INFO level log
     */
    void info(const QString& module, const QString& message);

    /**
     * @brief Shortcut method: write WARNING level log
     */
    void warning(const QString& module, const QString& message);

    /**
     * @brief Shortcut method: write ERROR level log
     */
    void error(const QString& module, const QString& message);

    /**
     * @brief Shortcut method: write FATAL level log
     */
    void fatal(const QString& module, const QString& message);

    /**
     * @brief Set minimum log level
     * @param level Minimum log level
     */
    void setMinLevel(LogLevel level);

    /**
     * @brief Set whether to capture Qt system messages
     * @param enable Whether to enable
     */
    void setCaptureQtMessages(bool enable);

    /**
     * @brief Clean up old log files
     */
    void cleanupOldFiles();

    /**
     * @brief Shutdown the logging system
     */
    void shutdown();

    /**
     * @brief Load configuration from config file
     * @param settings QSettings instance
     */
    void loadConfig(QSettings& settings);

    /**
     * @brief Save configuration to config file
     * @param settings QSettings instance
     */
    void saveConfig(QSettings& settings);

    /**
     * @brief Initialize logging system with current configuration
     * @return Whether initialization succeeded
     */
    bool initializeWithConfig();

    /**
     * @brief Get configuration value
     */
    QVariant getConfigValue(const QString& key, const QVariant& defaultValue = QVariant());

    /**
     * @brief Set configuration value
     */
    void setConfigValue(const QString& key, const QVariant& value);

signals:
    /**
     * @brief Log message signal (can be used for UI display)
     */
    void logMessage(const QString& message);

private:
    explicit ZLogger(QObject *parent = nullptr);
    ~ZLogger();

    // Disable copy and assignment
    ZLogger(const ZLogger&) = delete;
    ZLogger& operator=(const ZLogger&) = delete;

    /**
     * @brief Get log level string
     */
    QString levelToString(const LogLevel& level) const;

    /**
     * @brief Get log string to level
     */
    LogLevel stringToLevel(const QString& level) const;

    /**
     * @brief Get current log file name
     */
    QString getCurrentLogFileName() const;

    /**
     * @brief Roll over log file
     */
    bool rolloverLogFile();

    /**
     * @brief Check if file rollover is needed
     */
    bool needRollover() const;

    /**
     * @brief Convert Qt message type to log level
     */
    LogLevel qtMsgTypeToLogLevel(QtMsgType type) const;

    /**
     * @brief Extract module name from file path
     */
    QString extractModuleFromPath(const char *file) const;

    static ZLogger* m_instance;           // Singleton instance
    static QtMessageHandler m_oldHandler; // Original message handler

    QFile* m_logFile;                    // Log file
    QTextStream* m_textStream;           // Text stream
    QMutex m_mutex;                      // Mutex for thread safety
    QString m_logDir;                    // Log directory
    quint64 m_maxFileSize;               // Maximum file size (bytes)
    int m_maxFiles;                      // Maximum number of files
    LogLevel m_minLevel;                 // Minimum log level
    bool m_initialized;                  // Initialization flag
    bool m_captureQtMessages;            // Whether to capture Qt system messages

    QMap<QString, QVariant> m_config; // Container for storing configuration
};



#endif // ZLOGGER_H
