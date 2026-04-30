// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zjsonnetworkmanager.h"

#include <QNetworkRequest>

ZJsonNetworkManager::ZJsonNetworkManager(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
}

void ZJsonNetworkManager::fetchJson(const QUrl &url)
{
    // Cancel any in-flight request first
    cancelFetch();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");

    m_currentReply = m_manager->get(request);

    connect(m_currentReply, &QNetworkReply::finished,
            this, &ZJsonNetworkManager::onReplyFinished);
    connect(m_currentReply, &QNetworkReply::downloadProgress,
            this, &ZJsonNetworkManager::fetchProgress);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_currentReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &ZJsonNetworkManager::onReplyError);
#else
    connect(m_currentReply, &QNetworkReply::errorOccurred,
            this, &ZJsonNetworkManager::onReplyError);
#endif
}

void ZJsonNetworkManager::cancelFetch()
{
    if (!m_currentReply)
        return;

    // abort() triggers finished signal synchronously, which calls onReplyFinished()
    // onReplyFinished() sets m_currentReply = nullptr and calls deleteLater().
    // So we must save the pointer, then abort, and let onReplyFinished handle cleanup.
    m_currentReply->abort();
}

bool ZJsonNetworkManager::isFetching() const
{
    return m_currentReply != nullptr;
}

void ZJsonNetworkManager::onReplyFinished()
{
    if (!m_currentReply)
        return;

    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        emit fetchError(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    emit fetchFinished(data);
}

void ZJsonNetworkManager::onReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    if (!m_currentReply)
        return;

    emit fetchError(m_currentReply->errorString());
}
