// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZCONFIGUREMAINWINDOW_H
#define ZCONFIGUREMAINWINDOW_H

#include <QMainWindow>
#include <QString>

class ConfigureBuildWidget;

class ZConfigureMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ZConfigureMainWindow(QWidget *parent = nullptr, const QString &windowTitle = QString());
    ~ZConfigureMainWindow();

private:
    void setupUI();
    void createMenus();
    void loadWindowSettings();
    void saveWindowSettings();

    ConfigureBuildWidget *m_centralWidget;
};

#endif // ZCONFIGUREMAINWINDOW_H
