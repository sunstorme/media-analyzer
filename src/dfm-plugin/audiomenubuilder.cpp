// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "audiomenubuilder.h"
#include "mediaactionhandler.h"
#include "i18n.h"

#include <dfm-extension/menu/dfmextmenu.h>
#include <dfm-extension/menu/dfmextaction.h>

USING_DFMEXT_NAMESPACE

AudioMenuBuilder::AudioMenuBuilder(DFMExtMenuProxy *proxy, MediaActionHandler *handler)
    : m_proxy(proxy)
    , m_handler(handler)
{
}

bool AudioMenuBuilder::build(DFMExtMenu *parent, const QString &filePath,
                              const CachedMediaInfo &info)
{
    QList<ZFfprobe::StreamInfo> audioStreams;
    for (const auto &s : info.streams) {
        if (s.codecType == "audio")
            audioStreams.append(s);
    }

    addFormatInfo(parent, filePath);
    addStreamInfo(parent, filePath);
    addStreamSubmenu(parent, PLUGIN_TR("Frame Info"), audioStreams, "frame_info", filePath);
    addStreamSubmenu(parent, PLUGIN_TR("Packet Info"), audioStreams, "packet_info", filePath);
    addOperations(parent, filePath, audioStreams);

    return true;
}

void AudioMenuBuilder::addFormatInfo(DFMExtMenu *parent, const QString &filePath)
{
    auto *handler = m_handler;
    auto action = m_proxy->createAction();
    action->setText(PLUGIN_TR("Format Info").toStdString());
    action->registerTriggered([handler, filePath](DFMExtAction *, bool) {
        handler->showFormatInfo(filePath);
    });
    parent->addAction(action);
}

void AudioMenuBuilder::addStreamInfo(DFMExtMenu *parent, const QString &filePath)
{
    auto *handler = m_handler;
    auto action = m_proxy->createAction();
    action->setText(PLUGIN_TR("Audio Stream Info").toStdString());
    action->registerTriggered([handler, filePath](DFMExtAction *, bool) {
        handler->showStreamInfo(filePath);
    });
    parent->addAction(action);
}

void AudioMenuBuilder::addStreamSubmenu(DFMExtMenu *parent, const QString &label,
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
        QString streamLabel = PLUGIN_TR("Audio Stream %1").arg(stream.index);
        if (!stream.codecName.isEmpty()) {
            streamLabel += QString(" (%1)").arg(stream.codecName);
        }
        if (!stream.language.isEmpty()) {
            streamLabel += QString(" [%1]").arg(stream.language.toUpper());
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

void AudioMenuBuilder::addOperations(DFMExtMenu *parent, const QString &filePath,
                                       const QList<ZFfprobe::StreamInfo> &streams)
{
    auto *handler = m_handler;
    auto opsAction = m_proxy->createAction();
    opsAction->setText(PLUGIN_TR("Operations").toStdString());

    auto opsMenu = m_proxy->createMenu();
    opsAction->setMenu(opsMenu);

    for (const auto &stream : streams) {
        QString ext = stream.codecName == "aac" ? "aac" : "mp3";
        QString label = PLUGIN_TR("Extract Stream %1 as %2").arg(stream.index).arg(ext.toUpper());

        auto extractAction = m_proxy->createAction();
        extractAction->setText(label.toStdString());
        int idx = stream.index;
        extractAction->registerTriggered([handler, filePath, idx, ext](DFMExtAction *, bool) {
            handler->extractStream(filePath, idx, ext);
        });
        opsMenu->addAction(extractAction);
    }

    auto convertAction = m_proxy->createAction();
    convertAction->setText(PLUGIN_TR("Convert Format...").toStdString());
    convertAction->registerTriggered([handler, filePath](DFMExtAction *, bool) {
        handler->convertFormat(filePath, "");
    });
    opsMenu->addAction(convertAction);

    parent->addAction(opsAction);
}
