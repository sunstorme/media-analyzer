// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef COMMANDHISTORY_H
#define COMMANDHISTORY_H

#include <QObject>
#include <QStringList>
#include <QStringListModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class CommandHistory : public QObject
{
    Q_OBJECT

public:
    explicit CommandHistory(QObject *parent = nullptr);

    QStringList history() const;
    void addCommand(const QString &command);
    void clearHistory();
    QStringListModel *model() const;

private:
    void loadHistory();
    void saveHistory();
    QString configFilePath() const;

    QStringListModel *m_model;
    static const int MAX_HISTORY = 100;
};

#endif // COMMANDHISTORY_H
