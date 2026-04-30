// SPDX-FileCopyrightText: 2025 - 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QScreen>
#include <QFileDialog>
#include <QDockWidget>
#include <QMessageBox>
#include <QtConcurrent>
#include <QVBoxLayout>
#include <QFile>

#include "common/zffprobe.h"
#include "common/zwindowhelper.h"
#include "common/common.h"
#include "common/zlogger.h"
#include "common/zmediaplayermanager.h"

#include "widgets/tablefmtwg.h"
#include "widgets/jsonfmtwg.h"
#include "widgets/globalconfingwg.h"
#include "widgets/logwg.h"
#include "widgets/fileswg.h"
#include "widgets/progressdlg.h"
#include "widgets/exportwg.h"
#include "widgets/helpquerywg.h"
#include "widgets/configurebuildtool.h"
#include "widgets/formatwg.h"
#include "widgets/streamswg.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QList<QAction *> getMediaInfoAvailableActions();
    QList<QAction *> getFilesAvailableAction();
    QList<QMenu *> getMediaInfoAvailableMenus();

    void showBasicInfo(const QString& function, const QString& windwowTitle, const ZExtraInfo &extrainfo);
    void showMediaInfo(const QString filePath, const QString& function, const QString& windwowTitle, const ZExtraInfo &extrainfo);

private slots:
    void InitConnectation();
    void popBasicInfoWindow(const QString &title, const QString &info, const ZExtraInfo &extrainfo);
    void popMediaInfoWindow(const QString &title, const QString &info, const ZExtraInfo &extrainfo);
    void popMediaPropsWindow(const QString &filePath);
    void loadMediaProperties(const QString &filePath);
    void loadMediaPropertiesAsync(const QString &filePath);

    void createDockWidgets();
    void saveLayoutSettings();
    void restoreLayoutSettings();

    void slotMenuBasic_InfoTriggered(QAction *action);
    void slotMenuMedia_InfoTriggered(bool checked);
    void slotMenuFileTriggered(QAction *action);
    void slotMenuConfigTriggered(QAction *action);
    void slotMenuHelpTriggered(QAction *action);
    void slotMenuPlayTriggered(QAction *action);
    void slotMenuViewTriggered(QAction *action);

    void slotMenuViewAboutToShow();
    
    // Dynamic menu slots
    void slotMenuFramesAboutToShow();
    void slotMenuPacketsAboutToShow();
    void slotDynamicStreamActionTriggered();
    
    // Player related slot functions
    void slotPlayerInstallationChanged(const QString& playerKey, bool installed);
    void slotShowInstallPlayersDialog();

private:
    void getMenuAllActions(QMenu* menu, QList<QAction*>& actionList);
    QList<QAction*> getMenuAllActions(QMenu* menu);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    /**
     * @brief Handle player action
     * @param playerKey Player key name
     * @param filePath Media file path
     */
    void handlePlayerAction(const QString& playerKey, const QString& filePath);
    
    /**
     * @brief Update player menu states
     */
    void updatePlayerMenuStates();
    
    /**
     * @brief Update dynamic stream menus
     */
    void updateStreamMenus();
    void updateFramesMenu();
    void updatePacketsMenu();
    void updateSubMenu(QMenu* subMenu, const QList<ZFfprobe::StreamInfo>& streams, const QString& menuType, const QString& streamType);

    Ui::MainWindow *ui;

    ZFfprobe m_probe;
    ZMediaPlayerManager* m_playerManager;

    FilesWG &m_filesWG = FilesWG::instance();
    LogWG &m_logWG = LogWG::instance();
    FormatWG &m_formatWG = FormatWG::instance();
    StreamsWG &m_streamsWG = StreamsWG::instance();

    QDockWidget *m_filesWGDock = nullptr;
    QDockWidget *m_logWGDock = nullptr;
    QDockWidget *m_formatWGDock = nullptr;
    QDockWidget *m_streamsWGDock = nullptr;

    QMap<QAction*, QWidget*> m_actionWidgetMap;

    QString m_mediaFile;

    QString GEOMETRY_KEY = "mainWindowGeometry";
    QString STATE_KEY = "mainWindowState";
};
#endif // MAINWINDOW_H
