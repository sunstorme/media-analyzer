// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZJSONMAINWINDOW_H
#define ZJSONMAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QByteArray>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QFileInfo>

#include "commandhistory.h"
#include "zjsonnetworkmanager.h"
#include "common/zcommandexecutor.h"
#include "widgets/jsonfmtwg.h"
#include "component/zprogressbar.h"

class ZJsonResultWindow;

class ZJsonMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ZJsonMainWindow(QWidget *parent = nullptr, const QString &customTitle = QString());
    ~ZJsonMainWindow();

    void loadJsonData(const QByteArray &data);
    void fetchUrl(const QString &url);

public slots:
    void onExecuteClicked();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUI();
    void setupConnections();
    bool isLocalFile(const QString &input);
    void loadLocalFile(const QString &filePath);

    // UI - command launcher only
    QComboBox *m_commandInput;
    QPushButton *m_executeButton;
    QCompleter *m_completer;

    // Logic
    CommandHistory *m_history;
};

// Separate window: JSON viewer + stop + progress bar
class ZJsonResultWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ZJsonResultWindow(QWidget *parent = nullptr, const QString &customTitle = QString(), bool syncEnabled = false);
    ~ZJsonResultWindow();

    void loadJsonData(const QByteArray &data);
    void loadJsonFile(const QString &filePath);  // Load file and enable sync if syncEnabled is true
    void startCommand(const QString &command);
    void startFetch(const QUrl &url);
    void setRunning(bool running);
    void setStatusMessage(const QString &message);

signals:
    void windowClosed();

private slots:
    void onStopClicked();
    void onCommandOutput(const QString &command, const QString &output, int index);
    void onCommandFinished(const QString &command, int index, int exitCode, QProcess::ExitStatus exitStatus);
    void onAllCommandsFinished(bool success);
    void onFetchFinished(const QByteArray &data);
    void onFetchError(const QString &error);
    void onFetchProgress(qint64 received, qint64 total);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUI();
    void setupConnections();

    // File sync functionality
    void _setupSyncFile(const QString &filePath);  // Private implementation
    void saveToFile();
    void onFileChanged(const QString &path);
    void checkForExternalChanges();
    void showReloadConfirmation();
    void onDataChanged();

    // UI
    ZProgressBar *m_progressBar;
    QPushButton *m_stopButton;
    QLabel *m_statusLabel;
    JsonFormatWG *m_jsonViewer;

    // Logic
    ZCommandExecutor *m_executor;
    ZJsonNetworkManager *m_networkManager;

    // State
    QByteArray m_accumulatedOutput;
    bool m_isRunning = false;

    // File sync state
    bool m_syncEnabled = false;
    QString m_filePath;
    QFileSystemWatcher *m_fileWatcher;
    QTimer *m_autoSaveTimer;
    qint64 m_lastModified = 0;
    bool m_hasUnsavedChanges = false;
    QByteArray m_currentDataHash;  // Hash of current data for change detection
};

#endif // ZJSONMAINWINDOW_H
