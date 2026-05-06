// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zconfiguremainwindow.h"
#include "configurebuildwidget.h"
#include "zconfigureconfig.h"
#include "common/zwindowhelper.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QMessageBox>
#include <QKeySequence>

ZConfigureMainWindow::ZConfigureMainWindow(QWidget *parent, const QString &windowTitle)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
{
    setWindowTitle(windowTitle.isEmpty() ? tr("ZConfigure GUI") : windowTitle);

    setupUI();
    createMenus();
    loadWindowSettings();

    resize(1200, 900);
    ZWindowHelper::centerToCurrentScreen(this);
}

ZConfigureMainWindow::~ZConfigureMainWindow()
{
    saveWindowSettings();
}

void ZConfigureMainWindow::setupUI()
{
    m_centralWidget = new ConfigureBuildWidget(this);
    setCentralWidget(m_centralWidget);
}

void ZConfigureMainWindow::createMenus()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *exitAction = fileMenu->addAction(tr("E&xit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    QAction *aboutAction = helpMenu->addAction(tr("&About"));
    aboutAction->setStatusTip(tr("Show the application's about box"));
    connect(aboutAction, &QAction::triggered, [=]() {
        QMessageBox::about(this, tr("About ZConfigure GUI"),
            tr("<h3>ZConfigure GUI</h3>"
               "<p>Version 1.0.0</p>"
               "<p>Graphical configure/build tool for autotools projects.</p>"
               "<p>Copyright &copy; 2025 zhang hongyuan</p>"));
    });
}

void ZConfigureMainWindow::loadWindowSettings()
{
    ZConfigureConfig::WindowSettings settings = ZConfigureConfig::instance()->windowSettings();
    resize(settings.width, settings.height);
    if (!settings.splitterState.isEmpty()) {
        // Restore splitter state if needed
    }
}

void ZConfigureMainWindow::saveWindowSettings()
{
    ZConfigureConfig::WindowSettings settings;
    settings.width = width();
    settings.height = height();
    ZConfigureConfig::instance()->setWindowSettings(settings);
    ZConfigureConfig::instance()->save();
}
