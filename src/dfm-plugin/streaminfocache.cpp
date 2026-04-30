// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "streaminfocache.h"
#include "common/zlogger.h"

#include <QFileInfo>

StreamInfoCache &StreamInfoCache::instance()
{
    static StreamInfoCache cache;
    return cache;
}

StreamInfoCache::StreamInfoCache()
    : cache_(kMaxCacheSize)
{
}

QString StreamInfoCache::cacheKey(const QString &filePath) const
{
    QFileInfo info(filePath);
    return filePath + QString::number(info.lastModified().toMSecsSinceEpoch());
}

std::optional<CachedMediaInfo> StreamInfoCache::get(const QString &filePath)
{
    QMutexLocker locker(&mutex_);

    QString key = cacheKey(filePath);
    CachedMediaInfo *cached = cache_.object(key);

    if (!cached)
        return std::nullopt;

    if (cached->cachedAt.msecsTo(QDateTime::currentDateTime()) > kCacheTtlMs) {
        cache_.remove(key);
        return std::nullopt;
    }

    return *cached;
}

CachedMediaInfo StreamInfoCache::getOrProbe(const QString &filePath)
{
    auto cached = get(filePath);
    if (cached)
        return *cached;

    QFileInfo fileInfo(filePath);
    CachedMediaInfo info;
    info.fileMTime = fileInfo.lastModified().toMSecsSinceEpoch();
    info.cachedAt = QDateTime::currentDateTime();

    ZFfprobe probe;
    info.streams = probe.getMediaStreams(filePath);

    qDebug() << "Probed" << filePath << "found" << info.streams.size() << "streams";

    QMutexLocker locker(&mutex_);
    QString key = cacheKey(filePath);
    cache_.insert(key, new CachedMediaInfo(info));
    return info;
}

void StreamInfoCache::invalidate(const QString &filePath)
{
    QMutexLocker locker(&mutex_);
    QString key = cacheKey(filePath);
    cache_.remove(key);
}
