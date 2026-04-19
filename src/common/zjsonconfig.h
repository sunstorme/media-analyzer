// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZJSONCONFIG_H
#define ZJSONCONFIG_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileSystemWatcher>
#include <QMutex>
#include <QSettings>

/**
 * @brief JSON-based configuration manager with QSettings migration support
 *
 * Features:
 * - JSON file storage (same directory as QSettings)
 * - Automatic migration from QSettings (INI) to JSON
 * - File watcher for external changes with validation
 * - Thread-safe read/write
 * - Schema validation for known config sections
 * - Extensible design for adding new config groups
 */
class ZJsonConfig : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Get singleton instance
     */
    static ZJsonConfig* instance();

    /**
     * @brief Destroy singleton instance
     */
    static void destroy();

    /**
     * @brief Initialize config: load JSON or migrate from QSettings
     */
    void initialize();

    // ---- Value access ----

    /**
     * @brief Get a config value by dot-separated path
     * @param path Dot-separated path, e.g. "General.ffmpeg.activeToolGroup"
     * @param defaultValue Default value if key not found
     * @return The config value
     */
    QJsonValue getValue(const QString &path, const QJsonValue &defaultValue = QJsonValue()) const;

    /**
     * @brief Set a config value by dot-separated path
     * @param path Dot-separated path
     * @param value Value to set
     */
    void setValue(const QString &path, const QJsonValue &value);

    /**
     * @brief Get an entire config group as JSON object
     * @param group Group name (e.g. "Log", "General")
     * @return JSON object for the group
     */
    QJsonObject getGroup(const QString &group) const;

    /**
     * @brief Set an entire config group
     * @param group Group name
     * @param value JSON object for the group
     */
    void setGroup(const QString &group, const QJsonObject &value);

    /**
     * @brief Get the entire config as JSON document
     * @return JSON document
     */
    QJsonDocument getDocument() const;

    /**
     * @brief Replace the entire config from a JSON document (with validation)
     * @param doc New config document
     * @return true if validation passed and config was updated
     */
    bool setDocument(const QJsonDocument &doc);

    // ---- File operations ----

    /**
     * @brief Get the config file path
     */
    QString configFilePath() const;

    /**
     * @brief Save config to file
     */
    void saveToFile();

    /**
     * @brief Reload config from file (with validation)
     * @return true if reload succeeded
     */
    bool reloadFromFile();

    // ---- Migration ----

    /**
     * @brief Check if QSettings (INI) config exists
     */
    bool qSettingsExists() const;

    /**
     * @brief Migrate all config from QSettings to JSON
     * @return true if migration was performed
     */
    bool migrateFromQSettings();

    /**
     * @brief Check if JSON config file exists
     */
    bool jsonConfigExists() const;

    // ---- Validation ----

    /**
     * @brief Validate the entire config document against schema
     * @param doc Document to validate
     * @return true if valid
     */
    bool validate(const QJsonDocument &doc);

    /**
     * @brief Validate a specific group's data
     * @param group Group name
     * @param data Group data
     * @return true if valid
     */
    bool validateGroup(const QString &group, const QJsonObject &data);

    // ---- Default config ----

    /**
     * @brief Get the default config document
     */
    static QJsonDocument defaultConfig();

signals:
    /**
     * @brief Emitted when config is changed (either internally or externally)
     * @param path The path that changed, or empty for full reload
     */
    void configChanged(const QString &path);

    /**
     * @brief Emitted when external file change is detected
     */
    void externalConfigChanged();

    /**
     * @brief Emitted when config validation fails
     * @param errorMessage Description of the validation error
     */
    void validationError(const QString &errorMessage);

private:
    explicit ZJsonConfig(QObject *parent = nullptr);
    ~ZJsonConfig();

    // Singleton
    static ZJsonConfig *m_instance;
    static QMutex m_mutex;

    // Data
    QJsonObject m_root;
    QString m_filePath;
    QFileSystemWatcher *m_fileWatcher;
    mutable QMutex m_dataMutex;
    bool m_initialized;
    bool m_saving;  // flag to prevent self-triggered file watch

    // Internal methods
    void setupFilePath();
    void setupFileWatcher();
    void loadDefaults();
    QJsonValue getValueByPath(const QJsonObject &obj, const QString &path) const;
    void setValueByPath(QJsonObject &obj, const QString &path, const QJsonValue &value);
    void migrateGroupFromQSettings(QSettings &settings, const QString &group);
    void onFileChanged(const QString &path);

    // Validation helpers
    bool validateLogGroup(const QJsonObject &data);
    bool validateGeneralGroup(const QJsonObject &data);
};

#endif // ZJSONCONFIG_H
