// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zffplay.h"
#include <QFileInfo>
#include <QDebug>
#include <QProcessEnvironment>

#include "zfftoolconfig.h"

ZFFplay::ZFFplay(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_isPlaying(false)
{
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ZFFplay::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &ZFFplay::onProcessError);
}

ZFFplay::~ZFFplay()
{
    stop();
}

bool ZFFplay::playVideo(const QString &inputFile)
{
    if (!QFileInfo::exists(inputFile)) {
        qWarning() << "ZFFplay: Input file does not exist:" << inputFile;
        return false;
    }

    QStringList arguments;
    arguments << "-i" << inputFile
              << "-autoexit";  // Exit when playback finishes

    return executeCommand(arguments);
}

bool ZFFplay::playVideoWithOptions(const QString &inputFile, const QStringList &options)
{
    if (!QFileInfo::exists(inputFile)) {
        qWarning() << "ZFFplay: Input file does not exist:" << inputFile;
        return false;
    }

    QStringList arguments;
    arguments << "-i" << inputFile
              << options
              << "-autoexit";  // Exit when playback finishes

    return executeCommand(arguments);
}

bool ZFFplay::displayImage(const QString &imageFile)
{
    if (!QFileInfo::exists(imageFile)) {
        qWarning() << "ZFFplay: Image file does not exist:" << imageFile;
        return false;
    }

    QStringList arguments;
    arguments << "-loop" << "0"  // Loop infinitely
              << "-i" << imageFile;

    return executeCommand(arguments);
}

bool ZFFplay::displayImageWithSize(const QString &imageFile, int maxWidth, int maxHeight, int frameNumber, const QString &fileName)
{
    if (!QFileInfo::exists(imageFile)) {
        qWarning() << "ZFFplay: Image file does not exist:" << imageFile;
        return false;
    }

    QString windowTitle = "Image Preview";
    if (!fileName.isEmpty()) {
        windowTitle = QString("%1 - Frame %2 Preview").arg(fileName).arg(frameNumber);
    } else if (frameNumber >= 0) {
        windowTitle = QString("Frame %1 Preview").arg(frameNumber);
    }

    QStringList arguments;
    arguments << "-loop" << "0"  // Loop infinitely
              << "-i" << imageFile
              << "-vf" << QString("scale=min(%1\\,iw):-1").arg(maxWidth)  // Scale to max width, maintain aspect ratio
              << "-x" << QString::number(maxWidth)
              << "-y" << QString::number(maxHeight)
              << "-window_title" << windowTitle;

    return executeCommand(arguments);
}

void ZFFplay::stop()
{
    if (m_process && m_process->state() == QProcess::Running) {
        m_process->kill();
        m_isPlaying = false;
        qDebug() << "ZFFplay: Playback stopped";
    }
}

bool ZFFplay::isAvailable()
{
    QProcess process;
    process.start(ZFFToolConfig::ffplayPath(), QStringList() << "-version");
    if (!process.waitForStarted(3000)) {
        return false;
    }
    
    process.waitForFinished(3000);
    return process.exitCode() == 0;
}

bool ZFFplay::isPlaying() const
{
    return m_isPlaying;
}

void ZFFplay::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    
    m_isPlaying = false;
    
    if (exitCode == 0) {
        emit playbackFinished(exitCode);
    } else {
        QByteArray errorOutput = m_process->readAllStandardError();
        emit errorOccurred(QString("ffplay failed with exit code %1: %2")
                      .arg(exitCode)
                      .arg(QString::fromLocal8Bit(errorOutput)));
    }
}

void ZFFplay::onProcessError(QProcess::ProcessError error)
{
    m_isPlaying = false;
    QString errorString;
    
    switch (error) {
    case QProcess::FailedToStart:
        errorString = "Failed to start ffplay process";
        break;
    case QProcess::Crashed:
        errorString = "ffplay process crashed";
        break;
    case QProcess::Timedout:
        errorString = "ffplay process timed out";
        break;
    case QProcess::WriteError:
        errorString = "Write error to ffplay process";
        break;
    case QProcess::ReadError:
        errorString = "Read error from ffplay process";
        break;
    default:
        errorString = "Unknown ffplay process error";
        break;
    }
    
    qWarning() << "ZFFplay: Process error:" << errorString;
    emit errorOccurred(errorString);
}

bool ZFFplay::executeCommand(const QStringList &arguments)
{
    qDebug() << "ZFFplay: Starting ffplay with arguments:" << arguments.join(" ");
    
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QMap<QString, QString> envVars = ZFFToolConfig::environmentVariables();
        for (auto it = envVars.constBegin(); it != envVars.constEnd(); ++it) {
            env.insert(it.key(), it.value());
        }
        m_process->setProcessEnvironment(env);
    }
    m_process->start(ZFFToolConfig::ffplayPath(), arguments);
    
    if (!m_process->waitForStarted(5000)) {
        qWarning() << "ZFFplay: Failed to start ffplay process";
        emit errorOccurred("Failed to start ffplay process");
        return false;
    }
    
    m_isPlaying = true;
    emit playbackStarted();
    
    return true;
}
