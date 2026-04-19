// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zfftoolconfig.h"
#include "zjsonconfig.h"
#include "common.h"

#include <QDebug>

QString ZFFToolConfig::activeToolGroupId()
{
    return ZJsonConfig::instance()->getValue("General.ffmpeg.activeToolGroup", "system").toString();
}

void ZFFToolConfig::setActiveToolGroup(const QString &groupId)
{
    ZJsonConfig::instance()->setValue("General.ffmpeg.activeToolGroup", groupId);
}

ZFFToolGroup ZFFToolConfig::activeToolGroup()
{
    return getToolGroup(activeToolGroupId());
}

ZFFToolGroup ZFFToolConfig::getToolGroup(const QString &groupId)
{
    QJsonObject toolGroups = ZJsonConfig::instance()->getValue("General.ffmpeg.toolGroups").toObject();
    if (toolGroups.contains(groupId)) {
        return toolGroupFromJson(groupId, toolGroups.value(groupId).toObject());
    }

    // Return default group
    ZFFToolGroup defaultGroup;
    defaultGroup.id = groupId;
    defaultGroup.name = groupId;
    defaultGroup.ffprobePath = "ffprobe";
    defaultGroup.ffmpegPath = "ffmpeg";
    defaultGroup.ffplayPath = "ffplay";
    return defaultGroup;
}

QList<ZFFToolGroup> ZFFToolConfig::allToolGroups()
{
    QList<ZFFToolGroup> result;
    QJsonObject toolGroups = ZJsonConfig::instance()->getValue("General.ffmpeg.toolGroups").toObject();

    for (const QString &id : toolGroups.keys()) {
        result.append(toolGroupFromJson(id, toolGroups.value(id).toObject()));
    }

    return result;
}

QStringList ZFFToolConfig::toolGroupIds()
{
    QJsonObject toolGroups = ZJsonConfig::instance()->getValue("General.ffmpeg.toolGroups").toObject();
    return toolGroups.keys();
}

void ZFFToolConfig::setToolGroup(const ZFFToolGroup &group)
{
    QJsonObject toolGroups = ZJsonConfig::instance()->getValue("General.ffmpeg.toolGroups").toObject();
    toolGroups[group.id] = toolGroupToJson(group);
    ZJsonConfig::instance()->setValue("General.ffmpeg.toolGroups", toolGroups);
}

bool ZFFToolConfig::removeToolGroup(const QString &groupId)
{
    if (groupId == "system") {
        qWarning() << "ZFFToolConfig: Cannot remove the system tool group";
        return false;
    }

    QJsonObject toolGroups = ZJsonConfig::instance()->getValue("General.ffmpeg.toolGroups").toObject();
    if (!toolGroups.contains(groupId)) {
        return false;
    }

    toolGroups.remove(groupId);
    ZJsonConfig::instance()->setValue("General.ffmpeg.toolGroups", toolGroups);

    // If the removed group was active, switch to system
    if (activeToolGroupId() == groupId) {
        setActiveToolGroup("system");
    }

    return true;
}

bool ZFFToolConfig::hasToolGroup(const QString &groupId)
{
    QJsonObject toolGroups = ZJsonConfig::instance()->getValue("General.ffmpeg.toolGroups").toObject();
    return toolGroups.contains(groupId);
}

QString ZFFToolConfig::ffprobePath()
{
    return activeToolGroup().ffprobePath;
}

QString ZFFToolConfig::ffmpegPath()
{
    return activeToolGroup().ffmpegPath;
}

QString ZFFToolConfig::ffplayPath()
{
    return activeToolGroup().ffplayPath;
}

QMap<QString, QString> ZFFToolConfig::environmentVariables()
{
    return activeToolGroup().environmentVariables;
}

QStringList ZFFToolConfig::environmentStrings()
{
    QMap<QString, QString> envVars = environmentVariables();
    QStringList result;
    for (auto it = envVars.constBegin(); it != envVars.constEnd(); ++it) {
        result << QString("%1=%2").arg(it.key(), it.value());
    }
    return result;
}

ZFFToolGroup ZFFToolConfig::toolGroupFromJson(const QString &id, const QJsonObject &obj)
{
    ZFFToolGroup group;
    group.id = id;
    group.name = obj.value("name").toString(id);
    group.ffprobePath = obj.value("ffprobePath").toString("ffprobe");
    group.ffmpegPath = obj.value("ffmpegPath").toString("ffmpeg");
    group.ffplayPath = obj.value("ffplayPath").toString("ffplay");

    QJsonObject envObj = obj.value("environmentVariables").toObject();
    for (const QString &key : envObj.keys()) {
        group.environmentVariables[key] = envObj.value(key).toString();
    }

    return group;
}

QJsonObject ZFFToolConfig::toolGroupToJson(const ZFFToolGroup &group)
{
    QJsonObject obj;
    obj["name"] = group.name;
    obj["ffprobePath"] = group.ffprobePath;
    obj["ffmpegPath"] = group.ffmpegPath;
    obj["ffplayPath"] = group.ffplayPath;

    QJsonObject envObj;
    for (auto it = group.environmentVariables.constBegin(); it != group.environmentVariables.constEnd(); ++it) {
        envObj[it.key()] = it.value();
    }
    obj["environmentVariables"] = envObj;

    return obj;
}
