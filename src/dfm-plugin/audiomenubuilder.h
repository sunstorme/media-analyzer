// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef AUDIOMENUBUILDER_H
#define AUDIOMENUBUILDER_H

#include "mediamenubuilder.h"
#include <dfm-extension/menu/dfmextmenuproxy.h>

class MediaActionHandler;

class AudioMenuBuilder : public MediaMenuBuilder
{
public:
    AudioMenuBuilder(DFMEXT::DFMExtMenuProxy *proxy, MediaActionHandler *handler);
    bool build(DFMEXT::DFMExtMenu *parent, const QString &filePath,
               const CachedMediaInfo &info) override;
    QString category() const override { return "audio"; }

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

#endif // AUDIOMENUBUILDER_H
