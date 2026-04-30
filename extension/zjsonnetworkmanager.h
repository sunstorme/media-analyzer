// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZJSONNETWORKMANAGER_H
#define ZJSONNETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

class ZJsonNetworkManager : public QObject
{
    Q_OBJECT

public:
    explicit ZJsonNetworkManager(QObject *parent = nullptr);

    void fetchJson(const QUrl &url);
    void cancelFetch();
    bool isFetching() const;

signals:
    void fetchFinished(const QByteArray &data);
    void fetchError(const QString &errorMessage);
    void fetchProgress(qint64 received, qint64 total);

private slots:
    void onReplyFinished();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void onReplyError(QNetworkReply::NetworkError error);
#else
    void onReplyError(QNetworkReply::NetworkError error);
#endif

private:
    QNetworkAccessManager *m_manager;
    QNetworkReply *m_currentReply = nullptr;
};

#endif // ZJSONNETWORKMANAGER_H
