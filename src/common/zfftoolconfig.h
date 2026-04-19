// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZFFTOOLCONFIG_H
#define ZFFTOOLCONFIG_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QJsonObject>

/**
 * @brief FFmpeg tool group information
 */
struct ZFFToolGroup {
    QString id;              // Unique identifier (e.g. "system", "custom")
    QString name;            // Display name (e.g. "System FFmpeg")
    QString ffprobePath;     // Path to ffprobe executable
    QString ffmpegPath;      // Path to ffmpeg executable
    QString ffplayPath;      // Path to ffplay executable
    QMap<QString, QString> environmentVariables; // Environment variables

    bool isValid() const {
        return !id.isEmpty() && !name.isEmpty();
    }
};

/**
 * @brief Helper class for accessing FFmpeg tool configuration from ZJsonConfig
 *
 * This class provides a convenient interface for:
 * - Getting the active tool group
 * - Getting paths for ffprobe/ffmpeg/ffplay
 * - Getting environment variables for the active tool group
 * - Listing all available tool groups
 * - Adding/removing tool groups
 */
class ZFFToolConfig
{
public:
    /**
     * @brief Get the active tool group ID
     */
    static QString activeToolGroupId();

    /**
     * @brief Set the active tool group
     * @param groupId Tool group ID to activate
     */
    static void setActiveToolGroup(const QString &groupId);

    /**
     * @brief Get the active tool group info
     */
    static ZFFToolGroup activeToolGroup();

    /**
     * @brief Get a specific tool group by ID
     * @param groupId Tool group ID
     */
    static ZFFToolGroup getToolGroup(const QString &groupId);

    /**
     * @brief Get all available tool groups
     */
    static QList<ZFFToolGroup> allToolGroups();

    /**
     * @brief Get all tool group IDs
     */
    static QStringList toolGroupIds();

    /**
     * @brief Add or update a tool group
     * @param group Tool group info
     */
    static void setToolGroup(const ZFFToolGroup &group);

    /**
     * @brief Remove a tool group
     * @param groupId Tool group ID to remove
     * @return true if removed
     */
    static bool removeToolGroup(const QString &groupId);

    /**
     * @brief Check if a tool group exists
     */
    static bool hasToolGroup(const QString &groupId);

    // ---- Convenience methods for active tool paths ----

    /**
     * @brief Get ffprobe path from active tool group
     */
    static QString ffprobePath();

    /**
     * @brief Get ffmpeg path from active tool group
     */
    static QString ffmpegPath();

    /**
     * @brief Get ffplay path from active tool group
     */
    static QString ffplayPath();

    /**
     * @brief Get environment variables from active tool group
     */
    static QMap<QString, QString> environmentVariables();

    /**
     * @brief Get environment as QProcess-compatible format
     * @return List of "KEY=VALUE" strings
     */
    static QStringList environmentStrings();

private:
    static ZFFToolGroup toolGroupFromJson(const QString &id, const QJsonObject &obj);
    static QJsonObject toolGroupToJson(const ZFFToolGroup &group);
};

#endif // ZFFTOOLCONFIG_H