// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef STREAMINFOCACHE_H
#define STREAMINFOCACHE_H

#include <QString>
#include <QCache>
#include <QDateTime>
#include <QMutex>
#include <optional>
#include "common/zffprobe.h"

struct CachedMediaInfo
{
    QList<ZFfprobe::StreamInfo> streams;
    QDateTime cachedAt;
    qint64 fileMTime;
};

class StreamInfoCache
{
public:
    static StreamInfoCache &instance();

    std::optional<CachedMediaInfo> get(const QString &filePath);
    CachedMediaInfo getOrProbe(const QString &filePath);
    void invalidate(const QString &filePath);

private:
    StreamInfoCache();
    QString cacheKey(const QString &filePath) const;

    QCache<QString, CachedMediaInfo> cache_;
    QMutex mutex_;
    static constexpr int kMaxCacheSize = 50;
    static constexpr int kCacheTtlMs = 60000;
};

#endif // STREAMINFOCACHE_H
