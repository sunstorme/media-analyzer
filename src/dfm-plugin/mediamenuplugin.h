// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef MEDIAMENUPLUGIN_H
#define MEDIAMENUPLUGIN_H

#include <dfm-extension/menu/dfmextmenuplugin.h>
#include <dfm-extension/menu/dfmextaction.h>
#include <dfm-extension/menu/dfmextmenuproxy.h>
#include <dfm-extension/menu/dfmextmenu.h>
#include <QScopedPointer>

class MediaActionHandler;

class MediaMenuPlugin : public DFMEXT::DFMExtMenuPlugin
{
public:
    MediaMenuPlugin();
    ~MediaMenuPlugin();

    void initialize(DFMEXT::DFMExtMenuProxy *proxy) DFM_FAKE_OVERRIDE;
    bool buildNormalMenu(DFMEXT::DFMExtMenu *main,
                         const std::string &currentPath,
                         const std::string &focusPath,
                         const std::list<std::string> &pathList,
                         bool onDesktop) DFM_FAKE_OVERRIDE;
    bool buildEmptyAreaMenu(DFMEXT::DFMExtMenu *main,
                            const std::string &currentPath,
                            bool onDesktop) DFM_FAKE_OVERRIDE;

private:
    DFMEXT::DFMExtMenuProxy *m_proxy = nullptr;
    QScopedPointer<MediaActionHandler> m_actionHandler;
};

#endif // MEDIAMENUPLUGIN_H
