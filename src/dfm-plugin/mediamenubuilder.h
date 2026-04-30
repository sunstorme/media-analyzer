// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef MEDIAMENUBUILDER_H
#define MEDIAMENUBUILDER_H

#include <QString>
#include <dfm-extension/menu/dfmextmenu.h>
#include "streaminfocache.h"

class MediaMenuBuilder
{
public:
    virtual ~MediaMenuBuilder() = default;
    virtual bool build(DFMEXT::DFMExtMenu *parent, const QString &filePath,
                       const CachedMediaInfo &info) = 0;
    virtual QString category() const = 0;
};

#endif // MEDIAMENUBUILDER_H
