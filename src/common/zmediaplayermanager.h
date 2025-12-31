// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef MEDIAPLAYERMANAGER_H
#define MEDIAPLAYERMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QMessageBox>
#include "zmediaplayerconfig.h"

/**
 * @brief Media player management class
 * Responsible for player detection, installation and playback functionality
 */
class ZMediaPlayerManager : public QObject
{
    Q_OBJECT

public:
    explicit ZMediaPlayerManager(QObject *parent = nullptr);
    ~ZMediaPlayerManager();

    /**
     * @brief Check installation status of all players
     */
    void checkAllPlayers();

    /**
     * @brief Get list of installed players
     * @return List of installed player key names
     */
    QStringList getInstalledPlayers() const;

    /**
     * @brief Get list of all players
     * @return List of all player key names
     */
    QStringList getAllPlayers() const;

    /**
     * @brief Check if specified player is installed
     * @param playerKey Player key name
     * @return Whether installed
     */
    bool isPlayerInstalled(const QString& playerKey) const;

    /**
     * @brief Install specified player
     * @param playerKey Player key name
     * @param parent Parent window for displaying dialogs
     */
    void installPlayer(const QString& playerKey, QWidget *parent = nullptr);

    /**
     * @brief Play media file with specified player
     * @param playerKey Player key name
     * @param filePath Media file path
     * @return Whether playback started successfully
     */
    bool playWithPlayer(const QString& playerKey, const QString& filePath);

    /**
     * @brief Get player information
     * @param playerKey Player key name
     * @return Player information
     */
    ZMediaPlayerInfo getPlayerInfo(const QString& playerKey) const;

signals:
    /**
     * @brief Player installation status changed signal
     * @param playerKey Player key name
     * @param installed Whether installed
     */
    void playerInstallationChanged(const QString& playerKey, bool installed);

    /**
     * @brief Player installation started signal
     * @param playerKey Player key name
     */
    void playerInstallationStarted(const QString& playerKey);

    /**
     * @brief Player installation finished signal
     * @param playerKey Player key name
     * @param success Whether successful
     */
    void playerInstallationFinished(const QString& playerKey, bool success);

    /**
     * @brief Installation process output signal
     * @param output Output content
     */
    void installOutput(const QString& output);

    /**
     * @brief Installation process error signal
     * @param error Error content
     */
    void installError(const QString& error);

private slots:
    void onInstallProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onInstallProcessOutput();
    void onInstallProcessError();

private:
    /**
     * @brief Show installation confirmation dialog
     * @param playerKey Player key name
     * @param parent Parent window
     * @return Whether user confirmed installation
     */
    bool showInstallConfirmationDialog(const QString& playerKey, QWidget *parent);

    /**
     * @brief Show installation progress dialog
     * @param playerKey Player key name
     */
    void showInstallProgressDialog(const QString& playerKey);

    /**
     * @brief Close installation progress dialog
     */
    void closeInstallProgressDialog();

private:
    ZMediaPlayerConfig* m_config = nullptr;
    QProcess* m_installProcess = nullptr;
    QString m_currentInstallingPlayer;
    QMessageBox* m_progressDialog = nullptr;
    QString m_installOutput;
};

#endif // ZMEDIAPLAYERMANAGER_H
