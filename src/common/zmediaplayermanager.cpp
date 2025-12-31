// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zmediaplayermanager.h"
#include "widgets/progressdlg.h"
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QDebug>
#include <QProcessEnvironment>
#include <QDir>

ZMediaPlayerManager::ZMediaPlayerManager(QObject *parent)
    : QObject(parent)
    , m_config(ZMediaPlayerConfig::instance())
    , m_installProcess(nullptr)
    , m_progressDialog(nullptr)
{
    checkAllPlayers();
}

ZMediaPlayerManager::~ZMediaPlayerManager()
{
    if (m_installProcess) {
        m_installProcess->terminate();
        m_installProcess->deleteLater();
    }
    closeInstallProgressDialog();
}

void ZMediaPlayerManager::checkAllPlayers()
{
    QMap<QString, ZMediaPlayerInfo> players = m_config->getAllPlayers();
    for (const QString& playerKey : players.keys()) {
        m_config->isPlayerInstalled(playerKey);
    }
}

QStringList ZMediaPlayerManager::getInstalledPlayers() const
{
    QStringList installedPlayers;
    QMap<QString, ZMediaPlayerInfo> players = m_config->getAllPlayers();
    
    for (auto it = players.begin(); it != players.end(); ++it) {
        if (it.value().isInstalled) {
            installedPlayers << it.key();
        }
    }
    
    return installedPlayers;
}

QStringList ZMediaPlayerManager::getAllPlayers() const
{
    return m_config->getAllPlayers().keys();
}

bool ZMediaPlayerManager::isPlayerInstalled(const QString& playerKey) const
{
    return m_config->isPlayerInstalled(playerKey);
}

void ZMediaPlayerManager::installPlayer(const QString& playerKey, QWidget *parent)
{
    if (!m_config->getAllPlayers().contains(playerKey)) {
        qWarning() << "Unknown player:" << playerKey;
        return;
    }

    // Check if already installed
    if (m_config->isPlayerInstalled(playerKey)) {
        QMessageBox::information(parent, tr("Player Already Installed"),
                              tr("%1 is already installed, no need to reinstall.")
                              .arg(m_config->getPlayerInfo(playerKey).name));
        return;
    }

    // Show installation confirmation dialog
    if (!showInstallConfirmationDialog(playerKey, parent)) {
        return;
    }

    // If there's an installation process running, terminate it first
    if (m_installProcess && m_installProcess->state() == QProcess::Running) {
        m_installProcess->terminate();
        m_installProcess->deleteLater();
    }

    // Create new installation process
    m_installProcess = new QProcess(this);
    m_installProcess->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ZMediaPlayerManager::onInstallProcessFinished);
    connect(m_installProcess, &QProcess::readyReadStandardOutput,
            this, &ZMediaPlayerManager::onInstallProcessOutput);
    connect(m_installProcess, &QProcess::readyReadStandardError,
            this, &ZMediaPlayerManager::onInstallProcessError);

    m_currentInstallingPlayer = playerKey;
    m_installOutput.clear();

    // Show installation progress dialog
    showInstallProgressDialog(playerKey);

    // Start installation
    QString installCommand = m_config->getInstallCommand(playerKey);
    emit playerInstallationStarted(playerKey);
    
    // For installation commands, we still need bash because they may contain complex shell operations
    m_installProcess->start("bash", QStringList() << "-c" << installCommand);
    
    if (!m_installProcess->waitForStarted(3000)) {
        emit installError(tr("Failed to start installation process"));
        onInstallProcessFinished(-1, QProcess::CrashExit);
    }
}

bool ZMediaPlayerManager::playWithPlayer(const QString& playerKey, const QString& filePath)
{
    if (!m_config->getAllPlayers().contains(playerKey)) {
        qWarning() << "Unknown player:" << playerKey;
        return false;
    }

    // Check if file exists
    if (!QFile::exists(filePath)) {
        qWarning() << "File does not exist:" << filePath;
        return false;
    }

    // Check if player is installed
    if (!m_config->isPlayerInstalled(playerKey)) {
        qWarning() << "Player not installed:" << playerKey;
        return false;
    }

    // Get player info
    ZMediaPlayerInfo playerInfo = m_config->getPlayerInfo(playerKey);
    
    // Prepare arguments for the player
    QStringList arguments;
    
    // Special handling for different players to prevent fullscreen
    if (playerKey == PLAYER_VLC) {
        // VLC: windowed mode
    } else if (playerKey == PLAYER_MPV) {
        // MPV: windowed mode
    } else if (playerKey == PLAYER_FFPLAY) {
        // FFplay: windowed mode and autoexit
        arguments << "-autoexit" << "-window_title" << filePath + "-Media Analyzer";
    } else if (playerKey == PLAYER_SMPLAYER) {
        // SMPlayer: windowed mode
    } else if (playerKey == PLAYER_TOTEM) {
        // Totem: windowed mode
    } else if (playerKey == PLAYER_KMPLAYER) {
        // KMPlayer: windowed mode
    } else if (playerKey == PLAYER_XINE) {
        // Xine: windowed mode
    }
    
    // Add the file path as the last argument
    arguments << filePath;

    // Get current user environment variables
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // Remove some item under a debugger 
    env.remove("LD_LIBRARY_PATH");
    env.remove("QTDIR");

    // Create a process to set environment variables and start detached
    QProcess playProcess;
    playProcess.setProgram(playerInfo.executable);
    playProcess.setArguments(arguments);
    playProcess.setWorkingDirectory(QDir::currentPath());
    playProcess.setProcessEnvironment(env);

    qDebug() << env.toStringList();
    
    // Start the player directly without bash, with user environment
    bool success = playProcess.startDetached();
    
    if (success) {
        qDebug() << "Started playing with" << playerKey << ":" << filePath;
    } else {
        qWarning() << "Failed to start player" << playerKey;
    }

    return success;
}

ZMediaPlayerInfo ZMediaPlayerManager::getPlayerInfo(const QString& playerKey) const
{
    return m_config->getPlayerInfo(playerKey);
}

void ZMediaPlayerManager::onInstallProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    bool success = (exitCode == 0 && exitStatus == QProcess::NormalExit);
    
    if (success) {
        // Re-check player status
        m_config->isPlayerInstalled(m_currentInstallingPlayer);
        emit playerInstallationChanged(m_currentInstallingPlayer, true);
    }
    
    emit playerInstallationFinished(m_currentInstallingPlayer, success);
    
    // Close progress dialog
    closeInstallProgressDialog();
    
    // Show result
    if (m_progressDialog) {
        if (success) {
            m_progressDialog->setText(tr("%1 installation successful!")
                                     .arg(m_config->getPlayerInfo(m_currentInstallingPlayer).name));
        } else {
            m_progressDialog->setText(tr("%1 installation failed, please check logs.")
                                     .arg(m_config->getPlayerInfo(m_currentInstallingPlayer).name));
        }
    }
    
    // Cleanup
    if (m_installProcess) {
        m_installProcess->deleteLater();
        m_installProcess = nullptr;
    }
    
    m_currentInstallingPlayer.clear();
}

void ZMediaPlayerManager::onInstallProcessOutput()
{
    if (!m_installProcess) return;
    
    QString output = m_installProcess->readAllStandardOutput();
    if (!output.trimmed().isEmpty()) {
        m_installOutput += output;
        emit installOutput(output);
        
        // Update progress dialog
        if (m_progressDialog) {
            m_progressDialog->setText(tr("Installation output: %1").arg(output.trimmed()));
        }
    }
}

void ZMediaPlayerManager::onInstallProcessError()
{
    if (!m_installProcess) return;
    
    QString error = m_installProcess->readAllStandardError();
    if (!error.trimmed().isEmpty()) {
        m_installOutput += error;
        emit installError(error);
        
        // Update progress dialog
        if (m_progressDialog) {
            m_progressDialog->setText(tr("Error: %1").arg(error.trimmed()));
        }
    }
}

bool ZMediaPlayerManager::showInstallConfirmationDialog(const QString& playerKey, QWidget *parent)
{
    ZMediaPlayerInfo playerInfo = m_config->getPlayerInfo(playerKey);
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        parent,
        tr("Confirm Installation"),
        tr("Install %1?\n\nDescription: %2\n\nThis will use pkexec to request administrator privileges for software installation.")
        .arg(playerInfo.name)
        .arg(playerInfo.description),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes
    );
    
    return reply == QMessageBox::Yes;
}

void ZMediaPlayerManager::showInstallProgressDialog(const QString& playerKey)
{
    closeInstallProgressDialog();
    
    ZMediaPlayerInfo playerInfo = m_config->getPlayerInfo(playerKey);
    
    m_progressDialog = new QMessageBox();
    m_progressDialog->setWindowTitle(tr("Installing"));
    m_progressDialog->setText(tr("Installing %1, please wait...\n\nInstallation log:").arg(playerInfo.name));
    m_progressDialog->setStandardButtons(QMessageBox::Cancel);
    m_progressDialog->setModal(true);
    
    // Connect cancel button
    connect(m_progressDialog, &QMessageBox::buttonClicked, this, [this](QAbstractButton* button) {
        if (m_progressDialog->standardButton(button) == QMessageBox::Cancel) {
            if (m_installProcess && m_installProcess->state() == QProcess::Running) {
                m_installProcess->terminate();
            }
            closeInstallProgressDialog();
        }
    });
    
    m_progressDialog->show();
}

void ZMediaPlayerManager::closeInstallProgressDialog()
{
    if (m_progressDialog) {
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }
}
