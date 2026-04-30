// SPDX-FileCopyrightText: 2025 - 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "common/common.h"
#include "common/zjsonconfig.h"
#include "common/mediainfodisplayer.h"
#include <QDesktopServices>
#include <QFileInfo>
#include <QMetaObject>
#include <QTabWidget>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // Register ZExtraInfo type with Qt's meta-object system
    qRegisterMetaType<ZExtraInfo>("ZExtraInfo");
    
    ui->setupUi(this);

    setWindowTitle("FFprobe Viewer");

    ZWindowHelper::centerToCurrentScreen(this);
    setAttribute(Qt::WA_QuitOnClose, true);

    for (auto it : CONFIG_GROUPS) {
        auto action = new QAction(translatedConfigGroupName(it), ui->menuSetting);
        action->setObjectName(it);
        ui->menuSetting->addAction(action);
    }

    InitConnectation();

    // Create DockWidget layout
    createDockWidgets();

    // Restore layout settings
    restoreLayoutSettings();

    // Set window title
    setWindowTitle(APPLICATION_NAME);

    m_filesWG.addSubActions(tr("Media Info"), getMediaInfoAvailableActions());
    m_filesWG.addMenus(getMediaInfoAvailableMenus());
    m_filesWG.addSeparator();

    m_filesWG.addActions(getFilesAvailableAction());
    m_filesWG.addSeparator();

    m_filesWG.addMenus({ui->menuPlay});
    
    // If no file is selected in FilesWG, try to get from config
    QString currentFile = m_filesWG.getCurrentSelectFileName();

    // Load media properties for the current file on startup
    if (!currentFile.isEmpty() && QFile::exists(currentFile)) {
        loadMediaProperties(currentFile);
    }

    // Initialize player manager
    m_playerManager = new ZMediaPlayerManager(this);
    
    // Connect player manager signals
    connect(m_playerManager, &ZMediaPlayerManager::playerInstallationChanged,
            this, &MainWindow::slotPlayerInstallationChanged);

    m_actionWidgetMap = {
        {ui->actionLog, m_logWGDock},
        {ui->actionFiles, m_filesWGDock},
        {ui->actionFormat, m_formatWGDock},
        {ui->actionStreams, m_streamsWGDock}
    };
}

MainWindow::~MainWindow()
{
    delete ui;
}

QList<QAction *> MainWindow::getMediaInfoAvailableActions()
{
    QList<QAction *> tmpActions;

    tmpActions << ui->actionShow_Streams;
    tmpActions << ui->actionShow_Format;
    tmpActions << ui->actionShow_Chapters;

    return tmpActions;
}

QList<QAction *> MainWindow::getFilesAvailableAction()
{
    QList<QAction *> tmpActions;

    tmpActions << ui->actionOpen;
    tmpActions << ui->actionOpen_Files;
    tmpActions << ui->actionOpen_Folder;
    tmpActions << ui->actionExport;

    return tmpActions;
}

QList<QMenu *> MainWindow::getMediaInfoAvailableMenus()
{
    QList<QMenu *> tmpMenus;

    tmpMenus << ui->menuFrames;
    tmpMenus << ui->menuPackets;

    return tmpMenus;
}

void MainWindow::showBasicInfo(const QString &function, const QString &windwowTitle, const ZExtraInfo &extrainfo)
{
    bool sucess = false;
    QString retVal = m_probe.getBasicInfo(function, &sucess);

    if (!sucess) {
        qWarning() << tr("Basic info get error");
        return;
    }
    popBasicInfoWindow(windwowTitle,
                       retVal,
                       extrainfo);
}

void MainWindow::showMediaInfo(const QString filePath, const QString &function, const QString &windwowTitle, const ZExtraInfo &extrainfo)
{
    if (filePath != Common::instance()->getConfigValue(CURRENTFILE).toString()) {
        Common::instance()->setConfigValue(CURRENTFILE, filePath);
    }

    MediaInfoDisplayer::showMediaInfo(filePath, function, windwowTitle, extrainfo);
}

void MainWindow::InitConnectation()
{
    // menu connection
    connect(ui->menuFile, &QMenu::triggered, this, &MainWindow::slotMenuFileTriggered);
    connect(ui->menuBasic_Info, &QMenu::triggered, this, &MainWindow::slotMenuBasic_InfoTriggered);
    connect(ui->menuSetting, &QMenu::triggered, this, &MainWindow::slotMenuConfigTriggered);
    connect(ui->menuHelp, &QMenu::triggered, this, &MainWindow::slotMenuHelpTriggered);
    connect(ui->menuPlay, &QMenu::triggered, this, &MainWindow::slotMenuPlayTriggered);
    connect(ui->menuView, &QMenu::triggered, this, &MainWindow::slotMenuViewTriggered);

    connect(ui->menuView, &QMenu::aboutToShow, this, &MainWindow::slotMenuViewAboutToShow);
    
    // Connect dynamic menu signals
    connect(ui->menuFrames, &QMenu::aboutToShow, this, &MainWindow::slotMenuFramesAboutToShow);
    connect(ui->menuPackets, &QMenu::aboutToShow, this, &MainWindow::slotMenuPacketsAboutToShow);
    
    // media info
    QList<QAction*> mediaActions;
    getMenuAllActions(ui->menuMedia_Info, mediaActions);
    for (auto action : mediaActions){
        connect(action, &QAction::triggered, this, &MainWindow::slotMenuMedia_InfoTriggered);
    }

    // log
    connect(ZLogger::instance(), &ZLogger::logMessage, &m_logWG, &LogWG::outLog);

    connect(&m_filesWG, &FilesWG::currentFileActived, [=](QPair<QString, QString> filePair){
        // Update media properties dock if exists
        loadMediaPropertiesAsync(filePair.second);
    });
}

void MainWindow::popBasicInfoWindow(const QString &title, const QString &info, const ZExtraInfo &extrainfo)
{
    TableFormatWG *infoWindow = new TableFormatWG;
    // infoWindow->setObjectName(title.replace(" ", "") + "Wg");
    infoWindow->setAttribute(Qt::WA_DeleteOnClose);

    infoWindow->setWindowTitle(title);
    infoWindow->show();
    infoWindow->setExtraInfo(extrainfo);
    ZWindowHelper::centerToParent(infoWindow);
    infoWindow->initDetailTb(info, extrainfo.commandKey.toLower());

    // fit help option
    infoWindow->setHelpInfoKey(extrainfo.formatKey.mid(0, extrainfo.formatKey.length() - 1).toLower());

    qDebug() << title << info;
}

void MainWindow::popMediaInfoWindow(const QString &title, const QString &info, const ZExtraInfo &extrainfo)
{
    BaseFormatWG *mediaInfoWindow = nullptr;
    if (extrainfo.formatKey == FORMAT_JSON) {
        mediaInfoWindow = new JsonFormatWG;
    } else if (extrainfo.formatKey == FORMAT_TABLE) {
        mediaInfoWindow = new TableFormatWG;
    }

    if (mediaInfoWindow == nullptr)
        return;

    mediaInfoWindow->setExtraInfo(extrainfo);

    mediaInfoWindow->setWindowTitle(title);
    mediaInfoWindow->setAttribute(Qt::WA_DeleteOnClose);
    mediaInfoWindow->show();
    ZWindowHelper::centerToParent(mediaInfoWindow);
    mediaInfoWindow->loadJson(info.toUtf8());

    qDebug() << title << info.size();
}

void MainWindow::popMediaPropsWindow(const QString &filePath)
{
    // Update content if a valid file is provided
    if (!filePath.isEmpty()) {
        loadMediaProperties(filePath);
        
        // Update window title to reflect current file
        setWindowTitle(tr("%1 - Media Properties: %2").arg(APPLICATION_NAME, QFileInfo(filePath).fileName()));
    }
}

void MainWindow::loadMediaProperties(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    // Check if file exists before processing
    if (!QFile::exists(filePath)) {
        qWarning() << "Media file does not exist:" << filePath;
        return;
    }
    
    m_mediaFile = filePath;

    // Update dock titles
    QString fileName = QFileInfo(filePath).fileName();
    m_formatWGDock->setWindowTitle(tr("Format: %1").arg(fileName));
    m_streamsWGDock->setWindowTitle(tr("Streams: %1").arg(fileName));
    
    // Load format info
    QString formatData = m_probe.getMediaInfoJsonFormat(SHOW_FORMAT, m_mediaFile);
    m_formatWG.setFormatData(formatData.toUtf8());
    
    // Load streams info
    QString streamsData = m_probe.getMediaInfoJsonFormat(SHOW_STREAMS, m_mediaFile);
    m_streamsWG.setStreamsData(streamsData.toUtf8());
}

void MainWindow::loadMediaPropertiesAsync(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return;
    }

    // Skip if already loading the same file (prevents duplicate ProgressDialog
    // caused by multiple signals firing for a single click, e.g. pressed+clicked)
    if (m_mediaFile == filePath) {
        return;
    }

    // Check if file exists before processing
    if (!QFile::exists(filePath)) {
        qWarning() << "Media file does not exist:" << filePath;
        return;
    }
    
    m_mediaFile = filePath;

    // Update dock titles immediately
    QString fileName = QFileInfo(filePath).fileName();
    m_formatWGDock->setWindowTitle(tr("Format: %1").arg(fileName));
    m_streamsWGDock->setWindowTitle(tr("Streams: %1").arg(fileName));
    
    // Create and show progress dialog
    ProgressDialog *progressDlg = new ProgressDialog(this);
    progressDlg->setWindowTitle(tr("Loading Media Properties"));
    progressDlg->setProgressMode(ProgressDialog::Indeterminate);
    progressDlg->setMessage(tr("Loading media properties..."));
    progressDlg->setAutoClose(false);
    progressDlg->setCancelButtonVisible(false);
    progressDlg->show();
    
    // Run the loading operation in a separate thread
    QtConcurrent::run([=](){
        // Get format info
        QString formatInfo = m_probe.getMediaInfoJsonFormat(SHOW_FORMAT, m_mediaFile);
        
        // Get streams info
        QString streamsInfo = m_probe.getMediaInfoJsonFormat(SHOW_STREAMS, m_mediaFile);
        
        // Update both widgets in a single main thread invocation
        QMetaObject::invokeMethod(this, [this, formatInfo, streamsInfo, progressDlg]() {
            // Update format widget
            m_formatWG.setFormatData(formatInfo.toUtf8());
            
            // Update streams widget
            m_streamsWG.setStreamsData(streamsInfo.toUtf8());
            
            // Finish progress dialog
            progressDlg->finish();
            // Clean up progress dialog immediately
            progressDlg->deleteLater();
        }, Qt::QueuedConnection);
    });
}

void MainWindow::createDockWidgets()
{
    // Create FilesWG DockWidget (left side)
    m_filesWGDock = new QDockWidget(tr("Files"), this);
    m_filesWGDock->setObjectName("FilesDock");
    m_filesWGDock->setWidget(&m_filesWG);
    addDockWidget(Qt::LeftDockWidgetArea, m_filesWGDock);

    // Create LogWG DockWidget (bottom)
    m_logWGDock = new QDockWidget(tr("Logs"), this);
    m_logWGDock->setObjectName("LogsDock");
    m_logWGDock->setWidget(&m_logWG);
    addDockWidget(Qt::BottomDockWidgetArea, m_logWGDock);

    // Create FormatWG DockWidget (center area - central widget)
    m_formatWG.setObjectName("FormatWidget");
    m_formatWGDock = new QDockWidget(tr("Format"), this);
    m_formatWGDock->setObjectName("FormatDock");
    m_formatWGDock->setWidget(&m_formatWG);
    setCentralWidget(m_formatWGDock);

    // Create StreamsWG DockWidget (right side)
    m_streamsWG.setObjectName("StreamsWidget");
    m_streamsWGDock = new QDockWidget(tr("Streams"), this);
    m_streamsWGDock->setObjectName("StreamsDock");
    m_streamsWGDock->setWidget(&m_streamsWG);
    addDockWidget(Qt::RightDockWidgetArea, m_streamsWGDock);

    // Set dock widget sizes
    m_filesWGDock->setMinimumWidth(200);
    m_logWGDock->setMinimumHeight(150);
    m_formatWGDock->setMinimumWidth(300);
    m_streamsWGDock->setMinimumWidth(200);
}

void MainWindow::saveLayoutSettings()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    // Set encoding to UTF-8 to support Chinese paths (Qt5 only)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    settings.setIniCodec("UTF-8");
#endif
    settings.beginGroup(MAINWINDOW_SETTINGS_GROUP);
    // Save window geometry
    settings.setValue(GEOMETRY_KEY, saveGeometry());

    // Save dock layout state
    settings.setValue(STATE_KEY, saveState());
    settings.endGroup();
    // Sync immediately to ensure configuration is written
    settings.sync();
    qInfo() << "Window layout settings saved";
}

void MainWindow::restoreLayoutSettings()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    // Set encoding to UTF-8 to support Chinese paths (Qt5 only)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    settings.setIniCodec("UTF-8");
#endif
    settings.beginGroup(MAINWINDOW_SETTINGS_GROUP);
    // Restore window geometry
    if (settings.contains(GEOMETRY_KEY)) {
        restoreGeometry(settings.value(GEOMETRY_KEY).toByteArray());
    }

    // Restore dock layout state
    if (settings.contains(STATE_KEY)) {
        restoreState(settings.value(STATE_KEY).toByteArray());
        qInfo() << "Window layout restored from settings";
    } else {
        // Default layout for first run
        resize(1200, 800);
        qInfo() << "Using default window layout";
    }
    settings.endGroup();
}

void MainWindow::slotMenuBasic_InfoTriggered(QAction *action)
{
    if (!action) {
        return;
    }

    QString function = action->objectName().replace("action", "");
    ZExtraInfo info;

    info.commandKey = function;
    info.commandList << FFPROBE << ffmpegCommandList << function.toLower().prepend("-");
    info.formatKey = FORMAT_TABLE;

    showBasicInfo(function, info.commandList.join(" "), info);
}

void MainWindow::slotMenuMedia_InfoTriggered(bool checked)
{
    Q_UNUSED(checked);

    QAction *senderAction = static_cast<QAction*>(QObject::sender());

    if (!senderAction) {
        return;
    }

    ZExtraInfo extrainfo;
    QString function = senderAction->objectName().replace("action", "-").toLower();
    if (function.isEmpty()) {
        function = QObject::sender()->objectName();
    }

    if (QStringList{
            SHOW_FORMAT, SHOW_STREAMS, SHOW_CHAPTERS,
            SHOW_FRAMES, SHOW_PACKETS,
            SHOW_PROGRAMS,
            SHOW_VERSIONS, SHOW_PROGRAM_VERSION, SHOW_LIBRARY_VERSIONS,
            SHOW_PIXEL_FORMATS
        }.contains(function))
    {
        QString fileName = m_filesWG.getCurrentSelectFileName();
        if (!fileName.isEmpty()) {
            QString formats = m_probe.getMediaInfoJsonFormat(function, fileName);
            extrainfo.commandKey = function;
            extrainfo.formatKey = FORMAT_JSON;
            extrainfo.commandList << FFPROBE << ffmpegCommandList << function << OF << JSON << FI  << fileName;
            popMediaInfoWindow(extrainfo.commandList.join(" "), formats, extrainfo);
        } else {
            qWarning() << CURRENTFILE << "is empty, please retray";
        }

        return;
    }

    // Handle Format action
    if (senderAction == ui->actionFormat) {
        QString fileName = m_filesWG.getCurrentSelectFileName();
        if (!fileName.isEmpty()) {
            // Just make the dock widget visible and update its content
            popMediaPropsWindow(fileName);
        } else {
            qWarning() << CURRENTFILE << "is empty, please retry";
        }
        return;
    }

    if (QStringList{
            COUNT_FRAMES, COUNT_PACKETS
        }.contains(function)) {

        QString tmpFunction = function;

        if (function == COUNT_FRAMES) {
            tmpFunction = "-count_frames -show_entries stream=index,codec_type,nb_read_frames,nb_frames";
        }

        if (function == COUNT_PACKETS) {
            tmpFunction = "-count_packets -show_entries stream=index,codec_type,nb_packets,nb_read_packets";
        }

        QString fileName = m_filesWG.getCurrentSelectFileName();

        if (!fileName.isEmpty()) {
            extrainfo.commandKey = tmpFunction;
            extrainfo.formatKey = FORMAT_JSON;
            extrainfo.commandList << FFPROBE << ffmpegCommandList << tmpFunction << OF << JSON << FI  << fileName;
            showMediaInfo(fileName, tmpFunction, extrainfo.commandList.join(" "), extrainfo);
        } else {
            qWarning() << CURRENTFILE << fileName  << "is empty, please retray";
        }

        return;
    }
}

void MainWindow::slotMenuFileTriggered(QAction *action)
{
    if (!action) {
        return;
    }

    if (ui->actionOpen == action) {
        QString fileName = QFileDialog::getOpenFileName(
            nullptr,
            tr("Open File"),
            QDir::homePath(),
            tr("All Files (*.*)")
            );

        if (!fileName.isEmpty()) {
            qDebug() << "Selected file:" << fileName;
            Common::instance()->setConfigValue(CURRENTFILE, fileName);
            m_filesWG.addFileToHistory(fileName);
        }

        return;
    }

    if (ui->actionOpen_Files == action) {
        QStringList fileNames = QFileDialog::getOpenFileNames(
            nullptr,
            tr("Select Files"),
            QDir::homePath(),
            tr("All Files (*.*);;Media Files (*.mp4 *.mkv *.webm *.mp3);;Image Files (*.png *.jpg *.bmp)")
            );

        if (!fileNames.isEmpty()) {
            for (const QString &fileName : fileNames) {
                m_filesWG.addFileToHistory(fileName);
                qDebug() << "Selected file:" << fileName;
            }

            Common::instance()->setConfigValue(CURRENTFILES, fileNames);
        }

        return;
    }
    if (ui->actionExport == action) {
        ExportWG *exportDlg = new ExportWG;
        exportDlg->setWindowTitle(tr("Export Files"));
        exportDlg->setAttribute(Qt::WA_DeleteOnClose);

        if (m_filesWG.getCurrentSelectFileName().isEmpty()) {
            qWarning() << tr("No input file.");
            exportDlg->setExportModel(BasicInfo);
        } else {
            exportDlg->setExportModel(ExportModelType(BasicInfo | MediaInfo));

            exportDlg->setMediaInfoExportFiledsOptions(QStringList{
                SHOW_FORMAT, SHOW_STREAMS, SHOW_CHAPTERS,
                SHOW_FRAMES, SHOW_PACKETS,
                SHOW_PROGRAMS,
                SHOW_VERSIONS, SHOW_PROGRAM_VERSION, SHOW_LIBRARY_VERSIONS,
                SHOW_PIXEL_FORMATS
            });
        }

        exportDlg->setBasicInfoExportFiledsOptions(QStringList{
            VERSION, BUILDCONF, FORMATS, MUXERS, DEMUXERS,
            DEVICES, CODECS, DECODERS, ENCODERS, BSFS,
            PROTOCOLS, FILTERS, PIX_FMTS, LAYOUTS,
            SAMPLE_FMTS, COLORS
        });

        exportDlg->setInputMediaFilePath(m_filesWG.getCurrentSelectFileName());
        exportDlg->show();
        ZWindowHelper::centerToParent(exportDlg);
    }

}

void MainWindow::slotMenuConfigTriggered(QAction *action)
{
    if (!action) {
        return;
    }

    GlobalConfingWG *configWg = new GlobalConfingWG;
    ZWindowHelper::centerToParent(configWg, true);
    configWg->setCurrentConfig(action->objectName());
    configWg->show();
}

void MainWindow::slotMenuHelpTriggered(QAction *action)
{
    if (!action) {
        return;
    }

    if (ui->actionAbout == action) {
        QString packageVersion = Common::getCurrentPackageVersion();
        QString versionText = packageVersion.isEmpty() ? tr("Unknown") : packageVersion;
        
        QMessageBox::about(this,
                           tr("About MediaAnalyzer"),
                           tr("MediaAnalyzer\n"
                              "Version: %1\n"
                              "A powerful media file analysis tool that provides:\n"
                              "- Detailed media information display\n"
                              "- Multiple format views (JSON, Table)\n"
                              "- Real-time logging\n"
                              "- Customizable layout\n"
                              "- FFmpeg compilation tool\n\n").arg(versionText));
    }

    if (ui->actionApp_Dir == action) {
        QString appDir = QCoreApplication::applicationDirPath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(appDir));
    }

    if (ui->actionSetting_Dir == action) {
        // Prefer JSON config directory
        QString jsonConfigPath = ZJsonConfig::instance()->configFilePath();
        QFileInfo jsonFileInfo(jsonConfigPath);
        if (jsonFileInfo.exists()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(jsonFileInfo.absolutePath()));
        } else {
            // Fallback to QSettings path
            QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
            QFileInfo fileInfo(settings.fileName());
            if (fileInfo.exists()) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
            } else {
                qWarning() << "Setting dir not exists!";
            }
        }
    }

    if (ui->actionSetting_File == action) {
        // Prefer JSON config file
        QString jsonConfigPath = ZJsonConfig::instance()->configFilePath();
        QFile jsonFile(jsonConfigPath);
        if (jsonFile.exists()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(jsonConfigPath));
        } else {
            // Fallback to QSettings file
            QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
            QString fileName = settings.fileName();
            QFile file(fileName);
            if (file.exists()) {
                QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
            } else {
                qWarning() << "Setting File not exists!";
            }
        }
    }

    if (ui->actionHelp == action) {
        HelpQueryWg *helpWindow = new HelpQueryWg;
        helpWindow->setAttribute(Qt::WA_DeleteOnClose);

        helpWindow->setWindowTitle("Help Query");
        helpWindow->show();
        ZWindowHelper::centerToParent(helpWindow);
    }

    if (ui->actionFFmpeg_Builder == action) {
        ConfigureBuildTool *ffbuilder = new ConfigureBuildTool;
        ffbuilder->setAttribute(Qt::WA_DeleteOnClose);

        ffbuilder->setWindowTitle("Configure Builder");
        // ffbuilder->setMinimumSize(1200, 900);
        ffbuilder->show();
        ZWindowHelper::centerToParent(ffbuilder);
    }
}

void MainWindow::slotMenuPlayTriggered(QAction *action)
{
    if (!action) {
        return;
    }

    QString currentFile = m_filesWG.getCurrentSelectFileName();
    if (currentFile.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a media file first!"));
        return;
    }

    // Check if file exists
    if (!QFile::exists(currentFile)) {
        QMessageBox::warning(this, tr("Warning"), tr("Selected file does not exist:\n%1").arg(currentFile));
        return;
    }

    // Handle player menu items
    if (action == ui->actionVLC) {
        handlePlayerAction(PLAYER_VLC, currentFile);
    } else if (action == ui->actionMPV) {
        handlePlayerAction(PLAYER_MPV, currentFile);
    } else if (action == ui->actionFFplay) {
        handlePlayerAction(PLAYER_FFPLAY, currentFile);
    } else if (action == ui->actionSMPlayer) {
        handlePlayerAction(PLAYER_SMPLAYER, currentFile);
    } else if (action == ui->actionTotem) {
        handlePlayerAction(PLAYER_TOTEM, currentFile);
    } else if (action == ui->actionKMPlayer) {
        handlePlayerAction(PLAYER_KMPLAYER, currentFile);
    } else if (action == ui->actionXine) {
        handlePlayerAction(PLAYER_XINE, currentFile);
    } else if (action == ui->actionInstall_Players) {
        slotShowInstallPlayersDialog();
    }
}

void MainWindow::slotMenuViewTriggered(QAction *action)
{
    if (!action) {
        return;
    }

    QWidget *wg = m_actionWidgetMap.value(action);
    if (wg) {
        if (action->isChecked()) {
            wg->show();
            wg->raise();
        } else {
            wg->hide();
        }
    }
}

void MainWindow::slotMenuViewAboutToShow()
{
    for (auto action : getMenuAllActions(ui->menuView)) {
        action->setCheckable(true);
        action->setChecked(false);
        QWidget *wg = m_actionWidgetMap.value(action);
        if (wg && wg->isVisible()) {
            action->setChecked(true);
        }
    }
}

void MainWindow::getMenuAllActions(QMenu *menu, QList<QAction *> &actionList)
{
    if (!menu) return;

    QList<QAction*> actions = menu->actions();

    for (QAction* action : actions) {
        if (action->isSeparator()) {
            continue;
        }

        if (QMenu* subMenu = action->menu()) {
            getMenuAllActions(subMenu, actionList);
        } else {
            if (!actionList.contains(action)) {
                actionList.append(action);
            }
        }
    }
}

QList<QAction *> MainWindow::getMenuAllActions(QMenu *menu)
{
    QList<QAction *> actionList;

    getMenuAllActions(menu, actionList);

    return actionList;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveLayoutSettings();
    
    // Clean up image preview directory
    Common::cleanupDirectory(DEFAULT_IMAGE_PREVIEW_PATH);
    
    QMainWindow::closeEvent(event);
}

void MainWindow::handlePlayerAction(const QString& playerKey, const QString& filePath)
{
    if (!m_playerManager->isPlayerInstalled(playerKey)) {
        ZMediaPlayerInfo playerInfo = m_playerManager->getPlayerInfo(playerKey);
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Player Not Installed"),
            tr("%1 is not installed.\n\nInstall now?\n\nDescription: %2")
            .arg(playerInfo.name)
            .arg(playerInfo.description),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
        );
        
        if (reply == QMessageBox::Yes) {
            m_playerManager->installPlayer(playerKey, this);
        }
        return;
    }

    // Play file
    if (!m_playerManager->playWithPlayer(playerKey, filePath)) {
        ZMediaPlayerInfo playerInfo = m_playerManager->getPlayerInfo(playerKey);
        QMessageBox::warning(this, tr("Playback Failed"),
                         tr("Cannot play file with %1:\n%2")
                         .arg(playerInfo.name)
                         .arg(filePath));
    }
}

void MainWindow::slotPlayerInstallationChanged(const QString& playerKey, bool installed)
{
    Q_UNUSED(playerKey);
    Q_UNUSED(installed);
    
    // Can update menu status here, such as enabling/disabling menu items
    // Use timer delay here to ensure UI response
    QTimer::singleShot(100, [this]() {
        updatePlayerMenuStates();
    });
}

void MainWindow::slotShowInstallPlayersDialog()
{
    TableFormatWG *infoWindow = new TableFormatWG;
    infoWindow->setObjectName("PlayerInstallationStatusWg");
    infoWindow->setAttribute(Qt::WA_DeleteOnClose);

    QString title = tr("Player Installation Status");
    infoWindow->setWindowTitle(title);
    infoWindow->show();
    ZWindowHelper::centerToParent(infoWindow);

    QStringList headers;
    headers << tr("Player Name") << tr("Status") << tr("Description");
    
    QList<QStringList> playerData;
    QStringList allPlayers = m_playerManager->getAllPlayers();
    QStringList installedPlayers = m_playerManager->getInstalledPlayers();

    for (const QString& playerKey : allPlayers) {
        ZMediaPlayerInfo playerInfo = m_playerManager->getPlayerInfo(playerKey);
        QString status = installedPlayers.contains(playerKey) ? tr("Installed") : tr("Not Installed");
        
        QStringList rowData;
        rowData << playerInfo.name << status << playerInfo.description;
        playerData.append(rowData);
    }

    infoWindow->initHeaderDetailTb(headers);
    infoWindow->updateDataDetailTb(playerData);

    infoWindow->setHelpInfoKey("players");
    
    qDebug() << title << "Player installation status displayed in InfoWidgets";
}

void MainWindow::updatePlayerMenuStates()
{
    // Update status of player menu items
    QStringList installedPlayers = m_playerManager->getInstalledPlayers();
    
    ui->actionVLC->setEnabled(installedPlayers.contains(PLAYER_VLC) || true); // Always enabled to trigger installation
    ui->actionMPV->setEnabled(installedPlayers.contains(PLAYER_MPV) || true);
    ui->actionFFplay->setEnabled(installedPlayers.contains(PLAYER_FFPLAY) || true);
    ui->actionSMPlayer->setEnabled(installedPlayers.contains(PLAYER_SMPLAYER) || true);
    ui->actionTotem->setEnabled(installedPlayers.contains(PLAYER_TOTEM) || true);
    ui->actionKMPlayer->setEnabled(installedPlayers.contains(PLAYER_KMPLAYER) || true);
    ui->actionXine->setEnabled(installedPlayers.contains(PLAYER_XINE) || true);
}

void MainWindow::slotMenuFramesAboutToShow()
{
    updateFramesMenu();
}

void MainWindow::slotMenuPacketsAboutToShow()
{
    updatePacketsMenu();
}

void MainWindow::slotDynamicStreamActionTriggered()
{
    QAction *senderAction = static_cast<QAction*>(QObject::sender());
    if (!senderAction) {
        return;
    }
    
    QString fileName = m_filesWG.getCurrentSelectFileName();
    if (fileName.isEmpty()) {
        qWarning() << CURRENTFILE << "is empty, please retry";
        return;
    }
    
    QString objectName = senderAction->objectName();
    
    // Handle the original static actions (actionShow_Frames_Video, actionShow_Frames_Audio, etc.)
    if (objectName.contains("actionShow_Frames_Video") || objectName.contains("actionShow_Frames_Audio") ||
        objectName.contains("actionShow_Packets_Video") || objectName.contains("actionShow_Packets_Audio")) {
        
        QStringList parts = objectName.split('_', QT_SKIP_EMPTY_PARTS);
        if (parts.size() < 4) {
            qWarning() << "Invalid action object name:" << objectName;
            return;
        }
        
        QString type = parts[1]; // "Frames" or "Packets"
        QString codecType = parts[2]; // "Video" or "Audio"
        QString streamIndex = parts[3]; // "0"
        // Build the ffprobe command for default actions (use first stream of type)
        QString command = QString("%1 %2 %3").arg(SHOW_FRAMES)
                              .arg(SELECT_STREAMS)
                              .arg(streamIndex);
        ZExtraInfo extrainfo(command, FORMAT_TABLE);
        extrainfo.commandList << FFPROBE << ffmpegCommandList << command << OF << JSON << FI << fileName;
        MediaInfoDisplayer::showMediaInfo(fileName, command,
                      QString("[%1] - %2").arg(fileName).arg(extrainfo.commandList.join(" ")),
                      extrainfo);
        return;
    }
}

void MainWindow::updateStreamMenus()
{
    updateFramesMenu();
    updatePacketsMenu();
}

void MainWindow::updateSubMenu(QMenu* subMenu, const QList<ZFfprobe::StreamInfo>& streams, const QString& menuType, const QString& streamType)
{
    // Clear existing actions except separators
    QList<QAction*> actions = subMenu->actions();
    for (QAction* action : actions) {
        if (!action->isSeparator()) {
            subMenu->removeAction(action);
            action->deleteLater();
        }
    }
    
    // Add stream actions
    if (!streams.isEmpty()) {
        for (const ZFfprobe::StreamInfo& stream : streams) {
            QString actionText = tr("Stream %1").arg(stream.index);
            if (!stream.title.isEmpty()) {
                actionText += QString(" - %1").arg(stream.title);
            }
            if (streamType == "audio" && !stream.language.isEmpty()) {
                actionText += QString(" [%1]").arg(stream.language.toUpper());
            }
            if (!stream.codecName.isEmpty()) {
                actionText += QString(" (%1)").arg(stream.codecName);
            }
            
            QAction* action = new QAction(actionText, subMenu);
            action->setObjectName(QString("actionShow_%1_%2_%3").arg(menuType).arg(streamType).arg(stream.index));
            connect(action, &QAction::triggered, this, &MainWindow::slotDynamicStreamActionTriggered);
            subMenu->addAction(action);
        }
    }
}

void MainWindow::updateFramesMenu()
{
    QString fileName = m_filesWG.getCurrentSelectFileName();
    if (fileName.isEmpty()) {
        ui->menuFramesOfVideo->menuAction()->setVisible(false);
        ui->menuFramesOfAudio->menuAction()->setVisible(false);
        ui->menuFrames->menuAction()->setVisible(false);
        return;
    }

    // Get stream information
    QList<ZFfprobe::StreamInfo> streams = m_probe.getMediaStreams(fileName);

    // Group streams by type
    QList<ZFfprobe::StreamInfo> videoStreams;
    QList<ZFfprobe::StreamInfo> audioStreams;

    for (const ZFfprobe::StreamInfo& stream : streams) {
        if (stream.codecType == "video") {
            videoStreams.append(stream);
        } else if (stream.codecType == "audio") {
            audioStreams.append(stream);
        }
    }

    // Update Video sub-menu
    ui->menuFramesOfVideo->menuAction()->setVisible(!videoStreams.isEmpty());
    updateSubMenu(ui->menuFramesOfVideo, videoStreams, "Frames", "Video");

    // Update Audio sub-menu
    ui->menuFramesOfAudio->menuAction()->setVisible(!audioStreams.isEmpty());
    updateSubMenu(ui->menuFramesOfAudio, audioStreams, "Frames", "Audio");

    // Hide parent menu if no streams at all
    ui->menuFrames->menuAction()->setVisible(!videoStreams.isEmpty() || !audioStreams.isEmpty());
}

void MainWindow::updatePacketsMenu()
{
    QString fileName = m_filesWG.getCurrentSelectFileName();
    if (fileName.isEmpty()) {
        ui->menuPacketsOfVideo->menuAction()->setVisible(false);
        ui->menuPacketsOfAudio->menuAction()->setVisible(false);
        ui->menuPackets->menuAction()->setVisible(false);
        return;
    }

    // Get stream information
    QList<ZFfprobe::StreamInfo> streams = m_probe.getMediaStreams(fileName);

    // Group streams by type
    QList<ZFfprobe::StreamInfo> videoStreams;
    QList<ZFfprobe::StreamInfo> audioStreams;

    for (const ZFfprobe::StreamInfo& stream : streams) {
        if (stream.codecType == "video") {
            videoStreams.append(stream);
        } else if (stream.codecType == "audio") {
            audioStreams.append(stream);
        }
    }

    // Update Video sub-menu
    ui->menuPacketsOfVideo->menuAction()->setVisible(!videoStreams.isEmpty());
    updateSubMenu(ui->menuPacketsOfVideo, videoStreams, "Packets", "Video");

    // Update Audio sub-menu
    ui->menuPacketsOfAudio->menuAction()->setVisible(!audioStreams.isEmpty());
    updateSubMenu(ui->menuPacketsOfAudio, audioStreams, "Packets", "Audio");

    // Hide parent menu if no streams at all
    ui->menuPackets->menuAction()->setVisible(!videoStreams.isEmpty() || !audioStreams.isEmpty());
}
