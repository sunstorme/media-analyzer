// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zconfigureconfig.h"
#include <QFile>
#include <QJsonDocument>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#define ORGANIZATION_NAME "MediaTools"
#define APPLICATION_NAME "media-analyzer"
#define CONFIGURE_BUILDER_SETTINGS_GROUP "ConfigureBuilder"
#define CONFIGURE_BUILDER_RECENTFOLDERS_KEY "RecentFolders"

ZConfigureConfig *ZConfigureConfig::s_instance = nullptr;

ZConfigureConfig::ZConfigureConfig(QObject *parent)
    : QObject(parent)
{
    ensureConfigDirectory();
    m_configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/zconfigure-config.json";

    // If config doesn't exist, try to migrate from QSettings
    if (!QFile::exists(m_configPath)) {
        migrateFromQSettings();
    }

    load();
}

ZConfigureConfig* ZConfigureConfig::instance()
{
    if (!s_instance) {
        s_instance = new ZConfigureConfig();
    }
    return s_instance;
}

void ZConfigureConfig::ensureConfigDirectory()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (!QDir().mkpath(configDir)) {
        qWarning() << "Failed to create config directory:" << configDir;
    }
}

void ZConfigureConfig::migrateFromQSettings()
{
    QSettings oldSettings(ORGANIZATION_NAME, APPLICATION_NAME);
    oldSettings.beginGroup(CONFIGURE_BUILDER_SETTINGS_GROUP);

    QStringList oldPaths = oldSettings.value(CONFIGURE_BUILDER_RECENTFOLDERS_KEY).toStringList();
    if (!oldPaths.isEmpty()) {
        QJsonArray recentArray;
        for (const QString &path : oldPaths) {
            recentArray.append(path);
        }
        m_config["recentProjects"] = recentArray;
        m_config["version"] = "1.0";
        save();
        qDebug() << "Migrated configuration from QSettings to JSON";
    }

    oldSettings.endGroup();
}

bool ZConfigureConfig::load()
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open config file for reading:" << m_configPath;
        // Set default values
        m_config["version"] = "1.0";
        m_config["recentProjects"] = QJsonArray();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse config file:" << error.errorString();
        return false;
    }

    m_config = doc.object();
    return true;
}

bool ZConfigureConfig::save()
{
    QFile file(m_configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open config file for writing:" << m_configPath;
        return false;
    }

    QJsonDocument doc(m_config);
    file.write(doc.toJson());
    file.close();
    return true;
}

void ZConfigureConfig::addRecentProject(const QString& path)
{
    QJsonArray recent = m_config["recentProjects"].toArray();

    // Remove if already exists (to move it to top)
    for (int i = 0; i < recent.size(); ++i) {
        if (recent[i].toString() == path) {
            recent.removeAt(i);
            break;
        }
    }

    // Add to front
    recent.prepend(path);

    // Limit to 10 entries
    while (recent.size() > 10) {
        recent.removeAt(recent.size() - 1);
    }

    m_config["recentProjects"] = recent;
}

QStringList ZConfigureConfig::recentProjects() const
{
    QStringList result;
    QJsonArray recent = m_config["recentProjects"].toArray();
    for (const QJsonValue &val : recent) {
        result.append(val.toString());
    }
    return result;
}

ZConfigureConfig::WindowSettings ZConfigureConfig::windowSettings() const
{
    WindowSettings settings;
    QJsonObject winObj = m_config["windowSettings"].toObject();
    if (!winObj.isEmpty()) {
        settings.width = winObj["width"].toInt(1200);
        settings.height = winObj["height"].toInt(900);
        QString splitterStr = winObj["splitterState"].toString();
        settings.splitterState = QByteArray::fromBase64(splitterStr.toLatin1());
    }
    return settings;
}

void ZConfigureConfig::setWindowSettings(const WindowSettings& settings)
{
    QJsonObject winObj;
    winObj["width"] = settings.width;
    winObj["height"] = settings.height;
    winObj["splitterState"] = QString(settings.splitterState.toBase64());
    m_config["windowSettings"] = winObj;
}

ZConfigureConfig::BuildSettings ZConfigureConfig::buildSettings() const
{
    BuildSettings settings;
    QJsonObject buildObj = m_config["buildSettings"].toObject();
    if (!buildObj.isEmpty()) {
        settings.defaultJobs = buildObj["defaultJobs"].toInt(1);
        settings.makeOptions = buildObj["makeOptions"].toString();
    }
    return settings;
}

void ZConfigureConfig::setBuildSettings(const BuildSettings& settings)
{
    QJsonObject buildObj;
    buildObj["defaultJobs"] = settings.defaultJobs;
    buildObj["makeOptions"] = settings.makeOptions;
    m_config["buildSettings"] = buildObj;
}

ZConfigureConfig::ProjectInfo ZConfigureConfig::projectInfo(const QString& path) const
{
    ProjectInfo info;
    QJsonObject historyObj = m_config["projectHistory"].toObject();
    if (historyObj.contains(path)) {
        QJsonObject projObj = historyObj[path].toObject();
        QJsonArray optsArray = projObj["selectedOptions"].toArray();
        for (const QJsonValue &val : optsArray) {
            info.selectedOptions.append(val.toString());
        }
        QString timeStr = projObj["lastBuildTime"].toString();
        info.lastBuildTime = QDateTime::fromString(timeStr, Qt::ISODate);
    }
    return info;
}

void ZConfigureConfig::setProjectInfo(const QString& path, const ProjectInfo& info)
{
    QJsonObject historyObj = m_config["projectHistory"].toObject();
    QJsonObject projObj;
    QJsonArray optsArray;
    for (const QString &opt : info.selectedOptions) {
        optsArray.append(opt);
    }
    projObj["selectedOptions"] = optsArray;
    projObj["lastBuildTime"] = info.lastBuildTime.toString(Qt::ISODate);
    historyObj[path] = projObj;
    m_config["projectHistory"] = historyObj;
}
