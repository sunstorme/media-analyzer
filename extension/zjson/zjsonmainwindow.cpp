// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zjsonmainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QCompleter>
#include <QLineEdit>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QMessageBox>

#include "common/zwindowhelper.h"

// ============================================================
// ZJsonMainWindow — Launcher window (command input only)
// ============================================================

ZJsonMainWindow::ZJsonMainWindow(QWidget *parent, const QString &customTitle)
    : QMainWindow(parent)
    , m_history(new CommandHistory(this))
{
    setAcceptDrops(true);
    setupUI();
    if (!customTitle.isEmpty()) {
        setWindowTitle(customTitle);
    }
    setupConnections();
}

ZJsonMainWindow::~ZJsonMainWindow()
{
}

void ZJsonMainWindow::setupUI()
{
    setWindowTitle(tr("zjson-gui"));
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
    resize(600, 80);

    auto *centralWidget = new QWidget(this);
    auto *layout = new QHBoxLayout(centralWidget);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(4);

    m_commandInput = new QComboBox(this);
    m_commandInput->setEditable(true);
    m_commandInput->lineEdit()->setDragEnabled(false);
    m_commandInput->lineEdit()->setAcceptDrops(true);
    m_commandInput->lineEdit()->installEventFilter(this);
    m_commandInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_commandInput->setPlaceholderText(tr("Enter command, URL (http://, https://), or local file path..."));
    m_commandInput->setInsertPolicy(QComboBox::NoInsert);
    m_commandInput->addItems(m_history->history());

    m_completer = new QCompleter(m_history->model(), this);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setFilterMode(Qt::MatchContains);
    m_commandInput->setCompleter(m_completer);

    m_executeButton = new QPushButton(tr("Execute"), this);
    m_executeButton->setFixedWidth(80);
    m_executeButton->setDefault(true);

    layout->addWidget(m_commandInput);
    layout->addWidget(m_executeButton);

    setCentralWidget(centralWidget);

    ZWindowHelper::topCenterToCurrentScreen(this, 10);
}

void ZJsonMainWindow::setupConnections()
{
    connect(m_executeButton, &QPushButton::clicked,
            this, &ZJsonMainWindow::onExecuteClicked);
    connect(m_commandInput->lineEdit(), &QLineEdit::returnPressed,
            this, &ZJsonMainWindow::onExecuteClicked);
}

void ZJsonMainWindow::onExecuteClicked()
{
    QString input = m_commandInput->currentText().trimmed();
    if (input.isEmpty())
        return;

    m_history->addCommand(input);
    m_completer->setModel(m_history->model());

    auto *resultWindow = new ZJsonResultWindow();

    if (input.startsWith("http://") || input.startsWith("https://")) {
        resultWindow->startFetch(QUrl(input));
    } else if (isLocalFile(input)) {
        loadLocalFile(input);
        return;
    } else {
        resultWindow->startCommand(input);
    }

    resultWindow->setAttribute(Qt::WA_DeleteOnClose);
    ZWindowHelper::centerToCurrentScreen(resultWindow);
    resultWindow->show();
}

void ZJsonMainWindow::loadJsonData(const QByteArray &data)
{
    if (data.isEmpty())
        return;

    auto *resultWindow = new ZJsonResultWindow();
    resultWindow->setAttribute(Qt::WA_DeleteOnClose);
    resultWindow->loadJsonData(data);
    ZWindowHelper::centerToCurrentScreen(resultWindow);
    resultWindow->show();
}

void ZJsonMainWindow::fetchUrl(const QString &url)
{
    auto *resultWindow = new ZJsonResultWindow();
    resultWindow->setAttribute(Qt::WA_DeleteOnClose);
    resultWindow->startFetch(QUrl(url));
    ZWindowHelper::centerToCurrentScreen(resultWindow);
    resultWindow->show();
}

// ============================================================
// ZJsonResultWindow — JSON viewer + stop + progress bar
// ============================================================

ZJsonResultWindow::ZJsonResultWindow(QWidget *parent, const QString &customTitle, bool syncEnabled)
    : QMainWindow(parent)
    , m_executor(new ZCommandExecutor(this))
    , m_networkManager(new ZJsonNetworkManager(this))
    , m_syncEnabled(syncEnabled)
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_autoSaveTimer(new QTimer(this))
{
    setupUI();
    if (!customTitle.isEmpty()) {
        setWindowTitle(customTitle);
    }
    setupConnections();

    // Setup auto-save timer (started only when sync file is set)
    connect(m_autoSaveTimer, &QTimer::timeout, this, &ZJsonResultWindow::saveToFile);
    // Timer is started in setSyncFile()

    // Setup file watcher
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &ZJsonResultWindow::onFileChanged);

    // Connect data changed signal for unsaved changes tracking
    connect(m_jsonViewer, &JsonFormatWG::dataChanged,
            this, &ZJsonResultWindow::onDataChanged);

    setRunning(false);
}

ZJsonResultWindow::~ZJsonResultWindow()
{
    // Stop auto-save timer
    if (m_autoSaveTimer) {
        m_autoSaveTimer->stop();
    }
}

void ZJsonResultWindow::setupUI()
{
    setWindowTitle(tr("JSON Viewer"));
    resize(900, 600);

    auto *centralWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);

    // Top bar: progress bar + stop button (hidden when idle)
    auto *controlBar = new QHBoxLayout();
    controlBar->setSpacing(4);

    m_progressBar = new ZProgressBar(this);
    m_progressBar->setFixedHeight(6);
    m_progressBar->setTextVisible(false);
    m_progressBar->hide();

    m_stopButton = new QPushButton(tr("Stop"), this);
    m_stopButton->setFixedWidth(60);
    m_stopButton->hide();

    controlBar->addWidget(m_progressBar, 1);
    controlBar->addWidget(m_stopButton);

    mainLayout->addLayout(controlBar);

    // JSON viewer
    m_jsonViewer = new JsonFormatWG(this);
    mainLayout->addWidget(m_jsonViewer, 1);

    setCentralWidget(centralWidget);

    // Status bar
    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel, 1);
}

void ZJsonResultWindow::setupConnections()
{
    connect(m_stopButton, &QPushButton::clicked,
            this, &ZJsonResultWindow::onStopClicked);

    connect(m_executor, &ZCommandExecutor::commandOutput,
            this, &ZJsonResultWindow::onCommandOutput);
    connect(m_executor, &ZCommandExecutor::commandFinished,
            this, &ZJsonResultWindow::onCommandFinished);
    connect(m_executor, &ZCommandExecutor::allCommandsFinished,
            this, &ZJsonResultWindow::onAllCommandsFinished);

    connect(m_networkManager, &ZJsonNetworkManager::fetchFinished,
            this, &ZJsonResultWindow::onFetchFinished);
    connect(m_networkManager, &ZJsonNetworkManager::fetchError,
            this, &ZJsonResultWindow::onFetchError);
    connect(m_networkManager, &ZJsonNetworkManager::fetchProgress,
            this, &ZJsonResultWindow::onFetchProgress);
}

void ZJsonResultWindow::loadJsonData(const QByteArray &data)
{
    if (data.isEmpty())
        return;

    m_jsonViewer->loadData(data);
    m_statusLabel->setText(tr("Loaded %1 bytes").arg(data.size()));
}

void ZJsonResultWindow::loadJsonFile(const QString &filePath)
{
    // Load the file data first
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_statusLabel->setText(tr("Failed to open file: %1").arg(filePath));
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    if (!data.isEmpty()) {
        loadJsonData(data);
    }

    // Setup sync if enabled
    _setupSyncFile(filePath);
}

void ZJsonResultWindow::setStatusMessage(const QString &message)
{
    m_statusLabel->setText(message);
}

void ZJsonResultWindow::startCommand(const QString &command)
{
    m_accumulatedOutput.clear();
    setWindowTitle(command);
    setRunning(true);
    m_statusLabel->setText(tr("Executing: %1").arg(command));
    m_executor->executeCommands({command}, 1);
}

void ZJsonResultWindow::startFetch(const QUrl &url)
{
    setWindowTitle(url.toString());
    setRunning(true);
    m_statusLabel->setText(tr("Fetching: %1").arg(url.toString()));
    m_networkManager->fetchJson(url);
}

void ZJsonResultWindow::onStopClicked()
{
    if (m_executor->isRunning())
        m_executor->stopExecution();
    if (m_networkManager->isFetching())
        m_networkManager->cancelFetch();

    setRunning(false);
    m_statusLabel->setText(tr("Stopped"));
}

void ZJsonResultWindow::onCommandOutput(const QString &command, const QString &output, int index)
{
    Q_UNUSED(command)
    Q_UNUSED(index)
    m_accumulatedOutput.append(output.toUtf8());

    // Incremental parse: render as soon as valid JSON
    QJsonParseError err;
    QJsonDocument::fromJson(m_accumulatedOutput, &err);
    if (err.error == QJsonParseError::NoError)
        m_jsonViewer->loadData(m_accumulatedOutput);
}

void ZJsonResultWindow::onCommandFinished(const QString &command, int index, int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(command)
    Q_UNUSED(index)
    Q_UNUSED(exitStatus)

    if (exitCode != 0 && m_accumulatedOutput.isEmpty())
        m_statusLabel->setText(tr("Command exited with code %1 (no output)").arg(exitCode));
}

void ZJsonResultWindow::onAllCommandsFinished(bool success)
{
    if (!m_accumulatedOutput.isEmpty())
        loadJsonData(m_accumulatedOutput);

    setRunning(false);
    m_statusLabel->setText(success
        ? tr("Done")
        : tr("Finished with errors"));
}

void ZJsonResultWindow::onFetchFinished(const QByteArray &data)
{
    setRunning(false);
    loadJsonData(data);
    m_statusLabel->setText(tr("Done"));
}

void ZJsonResultWindow::onFetchError(const QString &error)
{
    setRunning(false);
    m_statusLabel->setText(tr("Error: %1").arg(error));
}

void ZJsonResultWindow::onFetchProgress(qint64 received, qint64 total)
{
    if (total > 0) {
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(static_cast<int>(received * 100 / total));
    }
}

void ZJsonResultWindow::setRunning(bool running)
{
    m_isRunning = running;

    // Progress bar and stop button: shown/hidden together
    m_progressBar->setVisible(running);
    m_stopButton->setVisible(running);

    if (running)
        m_progressBar->setRange(0, 0);  // Busy mode
}

void ZJsonResultWindow::closeEvent(QCloseEvent *event)
{
    if (m_isRunning)
        onStopClicked();
    // Save any unsaved changes before closing
    if (m_syncEnabled && !m_filePath.isEmpty() && m_hasUnsavedChanges) {
        saveToFile();
    }
    // Stop auto-save timer
    if (m_autoSaveTimer) {
        m_autoSaveTimer->stop();
    }
    event->accept();
}

// ============================================================
// ZJsonResultWindow - File sync functionality
// ============================================================

void ZJsonResultWindow::onDataChanged()
{
    if (m_syncEnabled && !m_filePath.isEmpty()) {
        m_hasUnsavedChanges = true;
    }
}

void ZJsonResultWindow::_setupSyncFile(const QString &filePath)
{
    // Validate file path
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        m_statusLabel->setText(tr("Invalid file: %1").arg(filePath));
        return;
    }

    m_filePath = filePath;
    m_lastModified = fileInfo.lastModified().toSecsSinceEpoch();

    // Initialize data hash from current data
    m_currentDataHash = m_jsonViewer->getData().toUtf8();
    m_hasUnsavedChanges = false;

    if (m_syncEnabled) {
        // Remove old path if exists
        if (!m_fileWatcher->files().isEmpty()) {
            m_fileWatcher->removePaths(m_fileWatcher->files());
        }
        m_fileWatcher->addPath(filePath);

        // Start auto-save timer
        if (!m_autoSaveTimer->isActive()) {
            m_autoSaveTimer->start(2000);  // Auto-save every 2 seconds
        }

        m_statusLabel->setText(tr("File: %1 (Sync enabled)").arg(filePath));
    } else {
        m_statusLabel->setText(tr("File: %1 (%2 bytes)").arg(filePath).arg(m_currentDataHash.size()));
    }
}

void ZJsonResultWindow::saveToFile()
{
    if (!m_syncEnabled || m_filePath.isEmpty())
        return;

    QString data = m_jsonViewer->getData();
    if (data.isEmpty())
        return;

    QByteArray dataBytes = data.toUtf8();

    // Check if data has changed
    if (dataBytes == m_currentDataHash) {
        return;  // Data unchanged, no need to save
    }

    // Check if file has been modified externally
    QFileInfo fileInfo(m_filePath);
    qint64 currentModified = fileInfo.lastModified().toSecsSinceEpoch();
    if (currentModified > m_lastModified) {
        // File was modified externally, don't overwrite
        checkForExternalChanges();
        return;
    }

    // Atomic write: write to temp file first
    QString tempPath = m_filePath + ".tmp";
    {
        QFile tempFile(tempPath);
        if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            m_statusLabel->setText(tr("Failed to save: %1").arg(m_filePath));
            return;
        }
        tempFile.write(dataBytes);
        tempFile.close();
    }

    // Atomic replace
    if (!QFile::rename(tempPath, m_filePath)) {
        QFile::remove(tempPath);
        m_statusLabel->setText(tr("Failed to save: %1").arg(m_filePath));
        return;
    }

    m_lastModified = QFileInfo(m_filePath).lastModified().toSecsSinceEpoch();
    m_currentDataHash = dataBytes;
    m_hasUnsavedChanges = false;
    m_statusLabel->setText(tr("Auto-saved: %1").arg(m_filePath));
}

void ZJsonResultWindow::onFileChanged(const QString &path)
{
    Q_UNUSED(path)
    if (!m_syncEnabled || m_filePath.isEmpty())
        return;

    // Delay to avoid multiple triggers
    QTimer::singleShot(500, this, &ZJsonResultWindow::checkForExternalChanges);
}

void ZJsonResultWindow::checkForExternalChanges()
{
    if (!m_syncEnabled || m_filePath.isEmpty())
        return;

    QFileInfo fileInfo(m_filePath);
    qint64 currentModified = fileInfo.lastModified().toSecsSinceEpoch();

    if (currentModified > m_lastModified) {
        showReloadConfirmation();
    }
}

void ZJsonResultWindow::showReloadConfirmation()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("File Changed Externally"),
        tr("The file '%1' has been modified externally.\n\nDo you want to reload the file?").arg(m_filePath),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        QFile file(m_filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();
            file.close();
            loadJsonData(data);
            m_lastModified = QFileInfo(m_filePath).lastModified().toSecsSinceEpoch();
            m_currentDataHash = data;  // Update data hash
            m_hasUnsavedChanges = false;
            m_statusLabel->setText(tr("Reloaded: %1").arg(m_filePath));
        }
    } else {
        // Update last modified to avoid asking again
        m_lastModified = QFileInfo(m_filePath).lastModified().toSecsSinceEpoch();
        m_hasUnsavedChanges = true;
    }
}

// ============================================================
// ZJsonMainWindow - File handling
// ============================================================

void ZJsonMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void ZJsonMainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl &url : urls) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                m_commandInput->setEditText(filePath);
                qWarning() << "Dropped file:" << filePath;
                onExecuteClicked();
                break;
            }
        }
    }
    event->acceptProposedAction();
}

bool ZJsonMainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_commandInput->lineEdit()) {
        if (event->type() == QEvent::DragEnter) {
            QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent*>(event);
            if (dragEvent->mimeData()->hasUrls()) {
                dragEvent->acceptProposedAction();
                return true;
            }
        } else if (event->type() == QEvent::Drop) {
            QDropEvent *dropEvent = static_cast<QDropEvent*>(event);
            const QMimeData *mimeData = dropEvent->mimeData();
            if (mimeData->hasUrls()) {
                QList<QUrl> urls = mimeData->urls();
                for (const QUrl &url : urls) {
                    if (url.isLocalFile()) {
                        QString filePath = url.toLocalFile();
                        m_commandInput->setEditText(filePath);
                        qWarning() << "Dropped file on lineEdit:" << filePath;
                        onExecuteClicked();
                        dropEvent->acceptProposedAction();
                        return true;
                    }
                }
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

bool ZJsonMainWindow::isLocalFile(const QString &input)
{
    QFileInfo fileInfo(input);
    return fileInfo.exists() && fileInfo.isFile();
}

void ZJsonMainWindow::loadLocalFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        auto *resultWindow = new ZJsonResultWindow();
        resultWindow->setAttribute(Qt::WA_DeleteOnClose);
        resultWindow->loadJsonData(QByteArray());
        resultWindow->setStatusMessage(tr("Failed to open file: %1").arg(filePath));
        ZWindowHelper::centerToCurrentScreen(resultWindow);
        resultWindow->show();
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    auto *resultWindow = new ZJsonResultWindow();
    resultWindow->setAttribute(Qt::WA_DeleteOnClose);
    resultWindow->loadJsonData(data);
    resultWindow->setStatusMessage(tr("File: %1 (%2 bytes)").arg(filePath).arg(data.size()));
    ZWindowHelper::centerToCurrentScreen(resultWindow);
    resultWindow->show();
}
