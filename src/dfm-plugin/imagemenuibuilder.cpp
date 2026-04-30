// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "imagemenuibuilder.h"
#include "mediaactionhandler.h"
#include "i18n.h"

#include <dfm-extension/menu/dfmextmenu.h>
#include <dfm-extension/menu/dfmextaction.h>

USING_DFMEXT_NAMESPACE

ImageMenuBuilder::ImageMenuBuilder(DFMExtMenuProxy *proxy, MediaActionHandler *handler)
    : m_proxy(proxy)
    , m_handler(handler)
{
}

bool ImageMenuBuilder::build(DFMExtMenu *parent, const QString &filePath,
                              const CachedMediaInfo &info)
{
    Q_UNUSED(info)

    auto *handler = m_handler;

    auto infoAction = m_proxy->createAction();
    infoAction->setText(PLUGIN_TR("Show Image Info").toStdString());
    infoAction->registerTriggered([handler, filePath](DFMExtAction *, bool) {
        handler->showImageInfo(filePath);
    });
    parent->addAction(infoAction);

    auto exifAction = m_proxy->createAction();
    exifAction->setText(PLUGIN_TR("Show EXIF Data").toStdString());
    exifAction->registerTriggered([handler, filePath](DFMExtAction *, bool) {
        handler->showImageExif(filePath);
    });
    parent->addAction(exifAction);

    auto convertAction = m_proxy->createAction();
    convertAction->setText(PLUGIN_TR("Convert Format...").toStdString());
    convertAction->registerTriggered([handler, filePath](DFMExtAction *, bool) {
        handler->convertFormat(filePath, "");
    });
    parent->addAction(convertAction);

    return true;
}
