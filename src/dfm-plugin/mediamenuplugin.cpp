// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "mediamenuplugin.h"
#include "mediaactionhandler.h"
#include "videomenubuilder.h"
#include "audiomenubuilder.h"
#include "imagemenuibuilder.h"
#include "filetypeutils.h"
#include "streaminfocache.h"
#include "i18n.h"

#include <dfm-extension/menu/dfmextmenu.h>
#include <dfm-extension/menu/dfmextmenuproxy.h>
#include <dfm-extension/menu/dfmextaction.h>

#include <QDebug>
#include <algorithm>

USING_DFMEXT_NAMESPACE

MediaMenuPlugin::MediaMenuPlugin()
    : DFMExtMenuPlugin()
    , m_actionHandler(new MediaActionHandler())
{
    registerInitialize([this](DFMExtMenuProxy *proxy) {
        initialize(proxy);
    });
    registerBuildNormalMenu([this](DFMExtMenu *main, const std::string &currentPath,
                                    const std::string &focusPath, const std::list<std::string> &pathList,
                                    bool onDesktop) {
        return buildNormalMenu(main, currentPath, focusPath, pathList, onDesktop);
    });
    registerBuildEmptyAreaMenu([this](DFMExtMenu *main, const std::string &currentPath, bool onDesktop) {
        return buildEmptyAreaMenu(main, currentPath, onDesktop);
    });
}

MediaMenuPlugin::~MediaMenuPlugin() = default;

void MediaMenuPlugin::initialize(DFMExtMenuProxy *proxy)
{
    m_proxy = proxy;
    qInfo() << "[MediaPlugin] initialized, proxy:" << (proxy ? "valid" : "null");
}

bool MediaMenuPlugin::buildNormalMenu(DFMExtMenu *main,
                                       const std::string &currentPath,
                                       const std::string &focusPath,
                                       const std::list<std::string> &pathList,
                                       bool onDesktop)
{
    qDebug() << "[MediaPlugin] buildNormalMenu called"
             << "onDesktop:" << onDesktop
             << "proxy:" << (m_proxy ? "yes" : "null")
             << "focusPath:" << QString::fromStdString(focusPath)
             << "pathList.size:" << pathList.size();

    if (onDesktop || !m_proxy) {
        qDebug() << "[MediaPlugin] early return: onDesktop or no proxy";
        return false;
    }

    // Only handle single file for now
    if (pathList.empty()) {
        qDebug() << "[MediaPlugin] early return: pathList empty";
        return false;
    }

    const QString filePath = QString::fromStdString(focusPath);
    MediaCategory category = FileTypeUtils::detectCategory(filePath);

    qDebug() << "[MediaPlugin] detected category:" << static_cast<int>(category)
             << "for file:" << filePath;

    if (category == MediaCategory::Unknown)
        return false;

    // Probe stream info (cached)
    CachedMediaInfo info = StreamInfoCache::instance().getOrProbe(filePath);

    // Create root action with category name
    auto rootAction = m_proxy->createAction();

    QString categoryName;
    switch (category) {
    case MediaCategory::Video: categoryName = PLUGIN_TR("Video Tools"); break;
    case MediaCategory::Audio: categoryName = PLUGIN_TR("Audio Tools"); break;
    case MediaCategory::Image: categoryName = PLUGIN_TR("Image Tools"); break;
    default: return false;
    }
    rootAction->setText(categoryName.toStdString());

    auto subMenu = m_proxy->createMenu();
    rootAction->setMenu(subMenu);

    // Select and run appropriate builder
    std::unique_ptr<MediaMenuBuilder> builder;
    switch (category) {
    case MediaCategory::Video: builder = std::make_unique<VideoMenuBuilder>(m_proxy, m_actionHandler.data()); break;
    case MediaCategory::Audio: builder = std::make_unique<AudioMenuBuilder>(m_proxy, m_actionHandler.data()); break;
    case MediaCategory::Image: builder = std::make_unique<ImageMenuBuilder>(m_proxy, m_actionHandler.data()); break;
    default: return false;
    }

    bool hasActions = builder->build(subMenu, filePath, info);

    if (!hasActions)
        return false;

    // Insert before "Send to" or append
    auto actions = main->actions();
    auto it = std::find_if(actions.cbegin(), actions.cend(), [](const auto *action) {
        const std::string &text = action->text();
        return (text.find("发送到") == 0) || (text.find("Send to") == 0);
    });

    if (it != actions.cend()) {
        main->insertAction(*it, rootAction);
    } else {
        main->addAction(rootAction);
    }

    return true;
}

bool MediaMenuPlugin::buildEmptyAreaMenu(DFMExtMenu *main,
                                          const std::string &currentPath,
                                          bool onDesktop)
{
    Q_UNUSED(main)
    Q_UNUSED(currentPath)
    Q_UNUSED(onDesktop)
    return false;
}
