// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
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

#include "common/zwindowhelper.h"

// ============================================================
// ZJsonMainWindow — Launcher window (command input only)
// ============================================================

ZJsonMainWindow::ZJsonMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_history(new CommandHistory(this))
{
    setupUI();
    setupConnections();
}

ZJsonMainWindow::~ZJsonMainWindow()
{
}

void ZJsonMainWindow::setupUI()
{
    setWindowTitle(tr("zjson-gui"));
    resize(600, 80);

    auto *centralWidget = new QWidget(this);
    auto *layout = new QHBoxLayout(centralWidget);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(4);

    m_commandInput = new QComboBox(this);
    m_commandInput->setEditable(true);
    m_commandInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_commandInput->setPlaceholderText(tr("Enter command or URL (http://, https://)..."));
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

ZJsonResultWindow::ZJsonResultWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_executor(new ZCommandExecutor(this))
    , m_networkManager(new ZJsonNetworkManager(this))
{
    setupUI();
    setupConnections();
    setRunning(false);
}

ZJsonResultWindow::~ZJsonResultWindow()
{
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
    event->accept();
}
