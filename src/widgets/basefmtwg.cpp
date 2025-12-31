// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "basefmtwg.h"

BaseFormatWG::BaseFormatWG(QWidget *parent)
    : QWidget{parent}, m_contextMenu(nullptr)
{
    createBaseContextMenu();
}

void BaseFormatWG::initialize()
{
    // Execute initialization steps in a fixed order
    initUI();
    initMenu();
    initShortCut();
    initConnection();
    initExtra();
}

bool BaseFormatWG::loadData(const QByteArray &jsonData) {
    return loadJson(jsonData);
}

void BaseFormatWG::setExtraInfo(const ZExtraInfo &info)
{
    m_extraInfo = info;
}

ZExtraInfo BaseFormatWG::getExtraInfo()
{
    return m_extraInfo;
}

void BaseFormatWG::addCustomMenuActions(QMenu *contextMenu)
{
    if (m_contextMenu) {
        m_contextMenu->addMenu(contextMenu);
    }
}

void BaseFormatWG::createBaseContextMenu()
{
    m_contextMenu = new QMenu(this);
}

void BaseFormatWG::showContextMenu(const QPoint &pos)
{
    if (!m_contextMenu) {
        createBaseContextMenu();
    }
    
    // Check if there is a valid selection
    if (hasValidSelection()) {
        m_contextMenu->exec(mapToGlobal(pos));
    }
}

void BaseFormatWG::addContextMenuSeparator()
{
    if (m_contextMenu) {
        m_contextMenu->addSeparator();
    }
}

void BaseFormatWG::addContextMenuAction(QAction *action)
{
    if (m_contextMenu && action) {
        m_contextMenu->addAction(action);
    }
}

void BaseFormatWG::addContextMenuMenu(QMenu *menu)
{
    if (m_contextMenu && menu) {
        m_contextMenu->addMenu(menu);
    }
}

void BaseFormatWG::removeContextMenuAction(QAction *action)
{
    if (m_contextMenu && action) {
        m_contextMenu->removeAction(action);
    }
}

void BaseFormatWG::removeContextMenuMenu(QMenu *menu)
{
    if (m_contextMenu && menu) {
        m_contextMenu->removeAction(menu->menuAction());
    }
}
