// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZCONFIGURECONFIG_H
#define ZCONFIGURECONFIG_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

class ZConfigureConfig : public QObject
{
    Q_OBJECT

public:
    struct WindowSettings {
        int width = 1200;
        int height = 900;
        QByteArray splitterState;
    };

    struct BuildSettings {
        int defaultJobs = 1;
        QString makeOptions;
    };

    struct ProjectInfo {
        QStringList selectedOptions;
        QDateTime lastBuildTime;
    };

    static ZConfigureConfig* instance();

    bool load();
    bool save();

    void addRecentProject(const QString& path);
    QStringList recentProjects() const;

    WindowSettings windowSettings() const;
    void setWindowSettings(const WindowSettings& settings);

    BuildSettings buildSettings() const;
    void setBuildSettings(const BuildSettings& settings);

    ProjectInfo projectInfo(const QString& path) const;
    void setProjectInfo(const QString& path, const ProjectInfo& info);

private:
    explicit ZConfigureConfig(QObject *parent = nullptr);
    ~ZConfigureConfig() = default;

    void migrateFromQSettings();
    void ensureConfigDirectory();

    QString m_configPath;
    QJsonObject m_config;
    static ZConfigureConfig *s_instance;
};

#endif // ZCONFIGURECONFIG_H
