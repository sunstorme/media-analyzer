// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZWINDOWHELPER_H
#define ZWINDOWHELPER_H

#include <QWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QStyle>

class ZWindowHelper
{
public:
    ZWindowHelper();

    // Center to primary screen
    static void centerToPrimaryScreen(QWidget* widget, bool dialog = false);

    // Center to current screen (where cursor is)
    static void centerToCurrentScreen(QWidget* widget, bool dialog = false);

    // Center to parent window
    static void centerToParent(QWidget* widget, bool dialog = false);

    // Horizontal center at top of current screen
    static void topCenterToCurrentScreen(QWidget* widget, int topMargin = 0);
};

#endif // ZWINDOWHELPER_H
