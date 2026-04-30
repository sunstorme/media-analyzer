// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "commandhistory.h"

#include <QFile>
#include <QDir>
#include <QStandardPaths>

CommandHistory::CommandHistory(QObject *parent)
    : QObject(parent)
    , m_model(new QStringListModel(this))
{
    loadHistory();
}

QStringList CommandHistory::history() const
{
    return m_model->stringList();
}

void CommandHistory::addCommand(const QString &command)
{
    if (command.trimmed().isEmpty())
        return;

    QStringList cmds = m_model->stringList();

    cmds.removeAll(command);
    cmds.prepend(command);

    while (cmds.size() > MAX_HISTORY)
        cmds.removeLast();

    m_model->setStringList(cmds);
    saveHistory();
}

void CommandHistory::clearHistory()
{
    m_model->setStringList(QStringList());
    saveHistory();
}

QStringListModel *CommandHistory::model() const
{
    return m_model;
}

QString CommandHistory::configFilePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(dir);
    return dir + "/command_history.json";
}

void CommandHistory::loadHistory()
{
    QFile file(configFilePath());
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject())
        return;

    QJsonArray arr = doc.object().value("commands").toArray();
    QStringList cmds;
    for (const QJsonValue &v : arr)
        cmds.append(v.toString());

    m_model->setStringList(cmds);
}

void CommandHistory::saveHistory()
{
    QJsonArray arr;
    for (const QString &cmd : m_model->stringList())
        arr.append(cmd);

    QJsonObject root;
    root["commands"] = arr;

    QFile file(configFilePath());
    if (!file.open(QIODevice::WriteOnly))
        return;

    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    file.close();
}
