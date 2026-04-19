// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zjsonconfig.h"
#include "common.h"
#include "common/qtcompat.h"
#include "zlogger.h"

#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QJsonParseError>
#include <QDebug>

ZJsonConfig *ZJsonConfig::m_instance = nullptr;
QMutex ZJsonConfig::m_mutex;

ZJsonConfig::ZJsonConfig(QObject *parent)
    : QObject(parent)
    , m_fileWatcher(nullptr)
    , m_initialized(false)
    , m_saving(false)
{
    setupFilePath();
}

ZJsonConfig::~ZJsonConfig()
{
    if (m_fileWatcher) {
        delete m_fileWatcher;
        m_fileWatcher = nullptr;
    }
}

ZJsonConfig *ZJsonConfig::instance()
{
    QMutexLocker locker(&m_mutex);
    if (!m_instance) {
        m_instance = new ZJsonConfig();
    }
    return m_instance;
}

void ZJsonConfig::destroy()
{
    QMutexLocker locker(&m_mutex);
    delete m_instance;
    m_instance = nullptr;
}

void ZJsonConfig::initialize()
{
    if (m_initialized) {
        return;
    }
    m_initialized = true;

    // Load defaults first
    loadDefaults();

    // Check if JSON config exists
    if (jsonConfigExists()) {
        // Load from JSON file
        if (!reloadFromFile()) {
            qWarning() << "ZJsonConfig: Failed to load JSON config, using defaults";
        }
    } else if (qSettingsExists()) {
        // Migrate from QSettings
        qDebug() << "ZJsonConfig: No JSON config found, migrating from QSettings...";
        if (migrateFromQSettings()) {
            qDebug() << "ZJsonConfig: Migration completed successfully";
            saveToFile();
        }
    } else {
        // First run - save defaults
        qDebug() << "ZJsonConfig: No config found, creating default config";
        saveToFile();
    }

    // Setup file watcher
    setupFileWatcher();
}

// ---- Value access ----

QJsonValue ZJsonConfig::getValue(const QString &path, const QJsonValue &defaultValue) const
{
    QMutexLocker locker(&m_dataMutex);
    QJsonValue val = getValueByPath(m_root, path);
    return val.isUndefined() ? defaultValue : val;
}

void ZJsonConfig::setValue(const QString &path, const QJsonValue &value)
{
    QMutexLocker locker(&m_dataMutex);
    setValueByPath(m_root, path, value);
    locker.unlock();

    saveToFile();
    emit configChanged(path);
}

QJsonObject ZJsonConfig::getGroup(const QString &group) const
{
    QMutexLocker locker(&m_dataMutex);
    return m_root.value(group).toObject();
}

void ZJsonConfig::setGroup(const QString &group, const QJsonObject &value)
{
    QMutexLocker locker(&m_dataMutex);
    m_root[group] = value;
    locker.unlock();

    saveToFile();
    emit configChanged(group);
}

QJsonDocument ZJsonConfig::getDocument() const
{
    QMutexLocker locker(&m_dataMutex);
    return QJsonDocument(m_root);
}

bool ZJsonConfig::setDocument(const QJsonDocument &doc)
{
    if (!validate(doc)) {
        qWarning() << "ZJsonConfig: Document validation failed";
        return false;
    }

    QMutexLocker locker(&m_dataMutex);
    m_root = doc.object();
    locker.unlock();

    saveToFile();
    emit configChanged(QString());
    return true;
}

// ---- File operations ----

QString ZJsonConfig::configFilePath() const
{
    return m_filePath;
}

void ZJsonConfig::saveToFile()
{
    QMutexLocker locker(&m_dataMutex);

    m_saving = true;
    QDir dir = QFileInfo(m_filePath).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QJsonDocument doc(m_root);
    QFile file(m_filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "ZJsonConfig: Config saved to" << m_filePath;
    } else {
        qWarning() << "ZJsonConfig: Failed to save config to" << m_filePath << ":" << file.errorString();
    }
    m_saving = false;
}

bool ZJsonConfig::reloadFromFile()
{
    QFile file(m_filePath);
    if (!file.exists()) {
        qWarning() << "ZJsonConfig: Config file does not exist:" << m_filePath;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ZJsonConfig: Failed to open config file:" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "ZJsonConfig: JSON parse error:" << parseError.errorString();
        emit validationError(tr("JSON parse error: %1").arg(parseError.errorString()));
        return false;
    }

    if (!validate(doc)) {
        qWarning() << "ZJsonConfig: Config validation failed";
        return false;
    }

    QMutexLocker locker(&m_dataMutex);
    m_root = doc.object();
    locker.unlock();

    emit configChanged(QString());
    return true;
}

// ---- Migration ----

bool ZJsonConfig::qSettingsExists() const
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    return !settings.allKeys().isEmpty();
}

bool ZJsonConfig::migrateFromQSettings()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    settings.setIniCodec("UTF-8");
#endif

    QStringList groups = settings.childGroups();
    qDebug() << "ZJsonConfig: Migrating groups:" << groups;

    for (const QString &group : groups) {
        migrateGroupFromQSettings(settings, group);
    }

    // Also migrate top-level keys (no group)
    settings.beginGroup(QString());
    QStringList topLevelKeys = settings.childKeys();
    for (const QString &key : topLevelKeys) {
        QMutexLocker locker(&m_dataMutex);
        m_root[key] = QJsonValue::fromVariant(settings.value(key));
    }
    settings.endGroup();

    return true;
}

bool ZJsonConfig::jsonConfigExists() const
{
    return QFile::exists(m_filePath);
}

// ---- Validation ----

bool ZJsonConfig::validate(const QJsonDocument &doc)
{
    if (doc.isNull() || doc.isEmpty()) {
        return false;
    }

    QJsonObject root = doc.object();

    // Validate known groups
    for (const QString &group : root.keys()) {
        QJsonObject groupData = root.value(group).toObject();
        if (!validateGroup(group, groupData)) {
            return false;
        }
    }

    return true;
}

bool ZJsonConfig::validateGroup(const QString &group, const QJsonObject &data)
{
    if (group == LOG_SETTINGS_GROUP) {
        return validateLogGroup(data);
    } else if (group == GENERAL_SETTINGS_GROUP) {
        return validateGeneralGroup(data);
    }
    // Unknown groups are allowed (extensibility)
    return true;
}

// ---- Default config ----

QJsonDocument ZJsonConfig::defaultConfig()
{
    QJsonObject root;

    // Log defaults
    QJsonObject log;
    log[LoggerConfig::ENABLED_KEY] = true;
    log[LoggerConfig::LEVEL_KEY] = 0;  // LOG_DEBUG
    log[LoggerConfig::DIRECTORY_KEY] = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/media-analyzer/logs";
    log[LoggerConfig::MAX_FILE_SIZE_KEY] = 10;
    log[LoggerConfig::MAX_FILES_KEY] = 5;
    log[LoggerConfig::CAPTURE_QT_MESSAGES_KEY] = true;
    log[LoggerConfig::FILE_NAME_PATTERN_KEY] = "media-analyzer_%1.log";
    log[LoggerConfig::LOG_FORMAT_KEY] = "[%1] [%2] [%3] %4 - %5";
    log[LoggerConfig::ENABLED_FILE] = true;
    log[LoggerConfig::ENABLED_LINE] = true;
    log[LoggerConfig::ENABLED_FUNCTION] = true;
    root[LOG_SETTINGS_GROUP] = log;

    // General defaults
    QJsonObject general;

    QJsonObject ffmpeg;
    ffmpeg["activeToolGroup"] = "system";

    QJsonObject systemGroup;
    systemGroup["name"] = "System FFmpeg";
    systemGroup["ffprobePath"] = "ffprobe";
    systemGroup["ffmpegPath"] = "ffmpeg";
    systemGroup["ffplayPath"] = "ffplay";

    QJsonObject systemEnv;
    systemGroup["environmentVariables"] = systemEnv;

    QJsonObject toolGroups;
    toolGroups["system"] = systemGroup;

    ffmpeg["toolGroups"] = toolGroups;
    general["ffmpeg"] = ffmpeg;

    root[GENERAL_SETTINGS_GROUP] = general;

    // MainWindow defaults (empty)
    root[MAINWINDOW_SETTINGS_GROUP] = QJsonObject();

    // WindowHead defaults (empty)
    root[WINDOWHEAD_SETTINGS_GROUP] = QJsonObject();

    // RecentFiles defaults
    QJsonObject recentFiles;
    recentFiles["files"] = QJsonArray();
    root[RECENTFILES_SETTINGS_GROUP] = recentFiles;

    // ConfigureBuilder defaults
    QJsonObject configureBuilder;
    configureBuilder[CONFIGURE_BUILDER_RECENTFOLDERS_KEY] = QJsonArray();
    root[CONFIGURE_BUILDER_SETTINGS_GROUP] = configureBuilder;

    // Version
    root["version"] = "1.0";

    return QJsonDocument(root);
}

// ---- Private methods ----

void ZJsonConfig::setupFilePath()
{
    // Use the same config directory as QSettings
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    m_filePath = configDir + "/" + ORGANIZATION_NAME + "/" + APPLICATION_NAME + ".json";
    qDebug() << "ZJsonConfig: Config file path:" << m_filePath;
}

void ZJsonConfig::setupFileWatcher()
{
    if (m_fileWatcher) {
        delete m_fileWatcher;
    }

    m_fileWatcher = new QFileSystemWatcher(this);

    // Watch the config file
    QFileInfo fi(m_filePath);
    if (fi.exists()) {
        m_fileWatcher->addPath(m_filePath);
    }

    // Also watch the directory (in case file is deleted/recreated)
    QString dirPath = fi.absolutePath();
    if (QDir(dirPath).exists()) {
        m_fileWatcher->addPath(dirPath);
    }

    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &ZJsonConfig::onFileChanged);
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged,
            this, [this](const QString &path) {
                Q_UNUSED(path);
                // Re-add file watch if file was recreated
                if (QFile::exists(m_filePath) && !m_fileWatcher->files().contains(m_filePath)) {
                    m_fileWatcher->addPath(m_filePath);
                }
            });
}

void ZJsonConfig::loadDefaults()
{
    QJsonDocument defaults = defaultConfig();
    QMutexLocker locker(&m_dataMutex);
    m_root = defaults.object();
}

QJsonValue ZJsonConfig::getValueByPath(const QJsonObject &obj, const QString &path) const
{
    QStringList parts = path.split('.', QT_SKIP_EMPTY_PARTS);
    QJsonValue current = obj;

    for (const QString &part : parts) {
        if (!current.isObject()) {
            return QJsonValue();
        }
        current = current.toObject().value(part);
        if (current.isUndefined()) {
            return QJsonValue();
        }
    }

    return current;
}

void ZJsonConfig::setValueByPath(QJsonObject &obj, const QString &path, const QJsonValue &value)
{
    QStringList parts = path.split('.', QT_SKIP_EMPTY_PARTS);
    if (parts.isEmpty()) {
        return;
    }

    // Navigate to the parent object, creating intermediate objects as needed
    QJsonObject *current = &obj;
    for (int i = 0; i < parts.size() - 1; ++i) {
        const QString &part = parts[i];
        if (!current->contains(part) || !current->value(part).isObject()) {
            current->insert(part, QJsonObject());
        }
        // Need to get a mutable reference - this is tricky with QJsonObject
        QJsonObject child = current->value(part).toObject();
        current->insert(part, child);
        // Re-fetch the mutable reference
        QJsonObject *temp = current;
        QJsonObject tempObj = temp->value(part).toObject();
        // We need to work with the nested object
        // Since QJsonObject doesn't give mutable refs, we rebuild from bottom up
    }

    // Actually, let's do it properly by rebuilding from the bottom up
    // Build the path from leaf to root
    QList<QJsonObject> objects;
    objects.append(obj);

    for (int i = 0; i < parts.size() - 1; ++i) {
        const QString &part = parts[i];
        QJsonObject currentObj = objects.last();
        QJsonObject child = currentObj.value(part).toObject();
        objects.append(child);
    }

    // Set the value at the leaf
    QJsonObject leaf = objects.last();
    leaf.insert(parts.last(), value);

    // Rebuild from leaf to root
    for (int i = parts.size() - 2; i >= 0; --i) {
        QJsonObject parent = objects[i];
        parent.insert(parts[i], leaf);
        leaf = parent;
    }

    obj = leaf;
}

void ZJsonConfig::migrateGroupFromQSettings(QSettings &settings, const QString &group)
{
    settings.beginGroup(group);
    QJsonObject groupObj;

    QStringList keys = settings.allKeys();
    for (const QString &key : keys) {
        QVariant value = settings.value(key);
        groupObj[key] = QJsonValue::fromVariant(value);
    }

    settings.endGroup();

    QMutexLocker locker(&m_dataMutex);
    m_root[group] = groupObj;
}

void ZJsonConfig::onFileChanged(const QString &path)
{
    Q_UNUSED(path);

    if (m_saving) {
        return;
    }

    qDebug() << "ZJsonConfig: External config change detected";

    // Re-add watch (some editors remove and recreate the file)
    if (QFile::exists(m_filePath) && !m_fileWatcher->files().contains(m_filePath)) {
        m_fileWatcher->addPath(m_filePath);
    }

    emit externalConfigChanged();

    // Try to reload with validation
    if (reloadFromFile()) {
        qDebug() << "ZJsonConfig: External config reloaded successfully";
    } else {
        qWarning() << "ZJsonConfig: External config reload failed, keeping current config";
    }
}

bool ZJsonConfig::validateLogGroup(const QJsonObject &data)
{
    // Log group validation - check types of known keys
    if (data.contains(LoggerConfig::ENABLED_KEY) && !data.value(LoggerConfig::ENABLED_KEY).isBool()) {
        emit validationError(tr("Log.%1 must be a boolean").arg(LoggerConfig::ENABLED_KEY));
        return false;
    }
    if (data.contains(LoggerConfig::LEVEL_KEY) && !data.value(LoggerConfig::LEVEL_KEY).isDouble()) {
        emit validationError(tr("Log.%1 must be a number").arg(LoggerConfig::LEVEL_KEY));
        return false;
    }
    if (data.contains(LoggerConfig::MAX_FILE_SIZE_KEY) && !data.value(LoggerConfig::MAX_FILE_SIZE_KEY).isDouble()) {
        emit validationError(tr("Log.%1 must be a number").arg(LoggerConfig::MAX_FILE_SIZE_KEY));
        return false;
    }
    if (data.contains(LoggerConfig::MAX_FILES_KEY) && !data.value(LoggerConfig::MAX_FILES_KEY).isDouble()) {
        emit validationError(tr("Log.%1 must be a number").arg(LoggerConfig::MAX_FILES_KEY));
        return false;
    }
    return true;
}

bool ZJsonConfig::validateGeneralGroup(const QJsonObject &data)
{
    // General.ffmpeg validation
    if (data.contains("ffmpeg")) {
        QJsonObject ffmpeg = data.value("ffmpeg").toObject();

        if (ffmpeg.contains("activeToolGroup") && !ffmpeg.value("activeToolGroup").isString()) {
            emit validationError(tr("General.ffmpeg.activeToolGroup must be a string"));
            return false;
        }

        if (ffmpeg.contains("toolGroups")) {
            if (!ffmpeg.value("toolGroups").isObject()) {
                emit validationError(tr("General.ffmpeg.toolGroups must be an object"));
                return false;
            }

            QJsonObject toolGroups = ffmpeg.value("toolGroups").toObject();
            for (const QString &key : toolGroups.keys()) {
                QJsonObject group = toolGroups.value(key).toObject();
                if (group.contains("ffprobePath") && !group.value("ffprobePath").isString()) {
                    emit validationError(tr("General.ffmpeg.toolGroups.%1.ffprobePath must be a string").arg(key));
                    return false;
                }
                if (group.contains("ffmpegPath") && !group.value("ffmpegPath").isString()) {
                    emit validationError(tr("General.ffmpeg.toolGroups.%1.ffmpegPath must be a string").arg(key));
                    return false;
                }
                if (group.contains("ffplayPath") && !group.value("ffplayPath").isString()) {
                    emit validationError(tr("General.ffmpeg.toolGroups.%1.ffplayPath must be a string").arg(key));
                    return false;
                }
                if (group.contains("environmentVariables") && !group.value("environmentVariables").isObject()) {
                    emit validationError(tr("General.ffmpeg.toolGroups.%1.environmentVariables must be an object").arg(key));
                    return false;
                }
            }
        }
    }
    return true;
}
