// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef VIDEOMENUBUILDER_H
#define VIDEOMENUBUILDER_H

#include "mediamenubuilder.h"
#include <dfm-extension/menu/dfmextmenuproxy.h>

class MediaActionHandler;

class VideoMenuBuilder : public MediaMenuBuilder
{
public:
    VideoMenuBuilder(DFMEXT::DFMExtMenuProxy *proxy, MediaActionHandler *handler);
    bool build(DFMEXT::DFMExtMenu *parent, const QString &filePath,
               const CachedMediaInfo &info) override;
    QString category() const override { return "video"; }

private:
    void addFormatInfo(DFMEXT::DFMExtMenu *parent, const QString &filePath);
    void addStreamInfo(DFMEXT::DFMExtMenu *parent, const QString &filePath);
    void addStreamSubmenu(DFMEXT::DFMExtMenu *parent, const QString &label,
                          const QList<ZFfprobe::StreamInfo> &streams,
                          const QString &actionPrefix, const QString &filePath);
    void addOperations(DFMEXT::DFMExtMenu *parent, const QString &filePath,
                       const QList<ZFfprobe::StreamInfo> &streams);

    DFMEXT::DFMExtMenuProxy *m_proxy;
    MediaActionHandler *m_handler;
};

#endif // VIDEOMENUBUILDER_H
