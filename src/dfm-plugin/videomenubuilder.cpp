// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "videomenubuilder.h"
#include "mediaactionhandler.h"
#include "i18n.h"

#include <dfm-extension/menu/dfmextmenu.h>
#include <dfm-extension/menu/dfmextaction.h>

#include <QDebug>

USING_DFMEXT_NAMESPACE

VideoMenuBuilder::VideoMenuBuilder(DFMExtMenuProxy *proxy, MediaActionHandler *handler)
    : m_proxy(proxy)
    , m_handler(handler)
{
}

bool VideoMenuBuilder::build(DFMExtMenu *parent, const QString &filePath,
                              const CachedMediaInfo &info)
{
    QList<ZFfprobe::StreamInfo> videoStreams;
    for (const auto &s : info.streams) {
        if (s.codecType == "video")
            videoStreams.append(s);
    }

    addFormatInfo(parent, filePath);
    addStreamInfo(parent, filePath);
    addStreamSubmenu(parent, PLUGIN_TR("Frame Info"), videoStreams, "frame_info", filePath);
    addStreamSubmenu(parent, PLUGIN_TR("Packet Info"), videoStreams, "packet_info", filePath);
    addOperations(parent, filePath, videoStreams);

    return true;
}

void VideoMenuBuilder::addFormatInfo(DFMExtMenu *parent, const QString &filePath)
{
    auto *handler = m_handler;
    auto action = m_proxy->createAction();
    action->setText(PLUGIN_TR("Format Info").toStdString());
    action->registerTriggered([handler, filePath](DFMExtAction *, bool) {
        handler->showFormatInfo(filePath);
    });
    parent->addAction(action);
}

void VideoMenuBuilder::addStreamInfo(DFMExtMenu *parent, const QString &filePath)
{
    auto *handler = m_handler;
    auto action = m_proxy->createAction();
    action->setText(PLUGIN_TR("Video Stream Info").toStdString());
    action->registerTriggered([handler, filePath](DFMExtAction *, bool) {
        handler->showStreamInfo(filePath);
    });
    parent->addAction(action);
}

void VideoMenuBuilder::addStreamSubmenu(DFMExtMenu *parent, const QString &label,
                                          const QList<ZFfprobe::StreamInfo> &streams,
                                          const QString &actionPrefix, const QString &filePath)
{
    if (streams.isEmpty())
        return;

    auto *handler = m_handler;
    auto menuAction = m_proxy->createAction();
    menuAction->setText(label.toStdString());

    auto subMenu = m_proxy->createMenu();
    menuAction->setMenu(subMenu);

    for (const auto &stream : streams) {
        QString streamLabel = PLUGIN_TR("Video Stream %1").arg(stream.index);
        if (!stream.codecName.isEmpty()) {
            streamLabel += QString(" (%1)").arg(stream.codecName);
        }

        auto streamAction = m_proxy->createAction();
        streamAction->setText(streamLabel.toStdString());

        QString actionId = actionPrefix;
        int idx = stream.index;
        streamAction->registerTriggered([handler, filePath, actionId, idx](DFMExtAction *, bool) {
            if (actionId == "frame_info") {
                handler->showFrameInfo(filePath, idx);
            } else {
                handler->showPacketInfo(filePath, idx);
            }
        });

        subMenu->addAction(streamAction);
    }

    parent->addAction(menuAction);
}

void VideoMenuBuilder::addOperations(DFMExtMenu *parent, const QString &filePath,
                                       const QList<ZFfprobe::StreamInfo> &streams)
{
    Q_UNUSED(streams)

    auto *handler = m_handler;
    auto opsAction = m_proxy->createAction();
    opsAction->setText(PLUGIN_TR("Operations").toStdString());

    auto opsMenu = m_proxy->createMenu();
    opsAction->setMenu(opsMenu);

    auto gifAction = m_proxy->createAction();
    gifAction->setText(PLUGIN_TR("Convert to GIF...").toStdString());
    gifAction->registerTriggered([handler, filePath](DFMExtAction *, bool) {
        handler->convertToGif(filePath);
    });
    opsMenu->addAction(gifAction);

    auto extractAudioAction = m_proxy->createAction();
    extractAudioAction->setText(PLUGIN_TR("Extract Audio Track").toStdString());
    extractAudioAction->registerTriggered([handler, filePath](DFMExtAction *, bool) {
        handler->extractStream(filePath, -1, "mp3");
    });
    opsMenu->addAction(extractAudioAction);

    auto convertAction = m_proxy->createAction();
    convertAction->setText(PLUGIN_TR("Convert Format...").toStdString());
    convertAction->registerTriggered([handler, filePath](DFMExtAction *, bool) {
        handler->convertFormat(filePath, "");
    });
    opsMenu->addAction(convertAction);

    parent->addAction(opsAction);
}
