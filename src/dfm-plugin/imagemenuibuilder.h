// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef IMAGEMENUBUILDER_H
#define IMAGEMENUBUILDER_H

#include "mediamenubuilder.h"
#include <dfm-extension/menu/dfmextmenuproxy.h>

class MediaActionHandler;

class ImageMenuBuilder : public MediaMenuBuilder
{
public:
    ImageMenuBuilder(DFMEXT::DFMExtMenuProxy *proxy, MediaActionHandler *handler);
    bool build(DFMEXT::DFMExtMenu *parent, const QString &filePath,
               const CachedMediaInfo &info) override;
    QString category() const override { return "image"; }

private:
    DFMEXT::DFMExtMenuProxy *m_proxy;
    MediaActionHandler *m_handler;
};

#endif // IMAGEMENUBUILDER_H
