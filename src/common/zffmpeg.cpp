// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zffmpeg.h"
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QCoreApplication>
#include <QProcessEnvironment>

#include "zfftoolconfig.h"

ZFFmpeg::ZFFmpeg(QObject *parent)
    : QObject(parent)
    , m_process(new QProcess(this))
{
}

ZFFmpeg::~ZFFmpeg()
{
    if (m_process) {
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
            m_process->waitForFinished(3000);
        }
    }
}

bool ZFFmpeg::extractFrame(const QString &inputFile, int frameNumber, const QString &outputFile)
{
    if (!QFileInfo::exists(inputFile)) {
        qWarning() << "ZFFmpeg: Input file does not exist:" << inputFile;
        return false;
    }

    QStringList arguments;
    arguments << "-i" << inputFile
              << "-vf" << QString("select=eq(n\\,%1)").arg(frameNumber)
              << "-vframes" << "1"
              << "-y"  // Overwrite output file
              << outputFile;

    qDebug() << "ZFFmpeg: Extracting frame" << frameNumber << "from" << inputFile
             << "to" << outputFile;
    qDebug() << "ZFFmpeg: Command: ffmpeg" << arguments.join(" ");

    return executeCommand(arguments);
}

bool ZFFmpeg::extractFrameAtTime(const QString &inputFile, const QString &time, const QString &outputFile)
{
    if (!QFileInfo::exists(inputFile)) {
        qWarning() << "ZFFmpeg: Input file does not exist:" << inputFile;
        return false;
    }

    QStringList arguments;
    arguments << "-i" << inputFile
              << "-ss" << time
              << "-vframes" << "1"
              << "-y"  // Overwrite output file
              << outputFile;

    qDebug() << "ZFFmpeg: Extracting frame at time" << time << "from" << inputFile
             << "to" << outputFile;
    qDebug() << "ZFFmpeg: Command: ffmpeg" << arguments.join(" ");

    return executeCommand(arguments);
}

QString ZFFmpeg::getVideoInfo(const QString &inputFile)
{
    if (!QFileInfo::exists(inputFile)) {
        qWarning() << "ZFFmpeg: Input file does not exist:" << inputFile;
        return QString();
    }

    QStringList arguments;
    arguments << "-i" << inputFile
              << "-hide_banner";

    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QMap<QString, QString> envVars = ZFFToolConfig::environmentVariables();
        for (auto it = envVars.constBegin(); it != envVars.constEnd(); ++it) {
            env.insert(it.key(), it.value());
        }
        m_process->setProcessEnvironment(env);
    }
    m_process->start(ZFFToolConfig::ffmpegPath(), arguments);
    
    if (!m_process->waitForStarted(5000)) {
        qWarning() << "ZFFmpeg: Failed to start ffmpeg process";
        return QString();
    }

    // Read stderr (ffmpeg outputs to stderr)
    QByteArray output;
    while (m_process->state() == QProcess::Running) {
        m_process->waitForReadyRead(100);
        output += m_process->readAllStandardError();
    }

    m_process->waitForFinished();
    output += m_process->readAllStandardError();

    return QString::fromLocal8Bit(output);
}

bool ZFFmpeg::isAvailable()
{
    QProcess process;
    process.start(ZFFToolConfig::ffmpegPath(), QStringList() << "-version");
    if (!process.waitForStarted(3000)) {
        return false;
    }
    
    process.waitForFinished(3000);
    return process.exitCode() == 0;
}

bool ZFFmpeg::executeCommand(const QStringList &arguments)
{
    // Ensure output directory exists
    QString outputFile = arguments.last();
    QFileInfo outputFileInfo(outputFile);
    QDir outputDir = outputFileInfo.absoluteDir();
    if (!outputDir.exists()) {
        if (!outputDir.mkpath(".")) {
            qWarning() << "ZFFmpeg: Failed to create output directory:" << outputDir.absolutePath();
            return false;
        }
    }

    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QMap<QString, QString> envVars = ZFFToolConfig::environmentVariables();
        for (auto it = envVars.constBegin(); it != envVars.constEnd(); ++it) {
            env.insert(it.key(), it.value());
        }
        m_process->setProcessEnvironment(env);
    }
    m_process->start(ZFFToolConfig::ffmpegPath(), arguments);
    
    if (!m_process->waitForStarted(5000)) {
        qWarning() << "ZFFmpeg: Failed to start ffmpeg process";
        return false;
    }

    // Wait for completion with timeout
    if (!m_process->waitForFinished(30000)) { // 30 seconds timeout
        qWarning() << "ZFFmpeg: ffmpeg process timed out";
        m_process->kill();
        m_process->waitForFinished(3000);
        return false;
    }

    if (m_process->exitCode() != 0) {
        QByteArray errorOutput = m_process->readAllStandardError();
        qWarning() << "ZFFmpeg: ffmpeg failed with exit code" << m_process->exitCode();
        qWarning() << "ZFFmpeg: Error output:" << QString::fromLocal8Bit(errorOutput);
        return false;
    }

    // Check if output file was created
    if (!QFileInfo::exists(outputFile)) {
        qWarning() << "ZFFmpeg: Output file was not created:" << outputFile;
        return false;
    }

    qDebug() << "ZFFmpeg: Frame extraction completed successfully";
    return true;
}

QString ZFFmpeg::generateOutputFilename(const QString &inputFile, int frameNumber)
{
    QFileInfo inputFileInfo(inputFile);
    QString baseName = inputFileInfo.baseName();
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    
    return QDir(tempPath).absoluteFilePath(QString("%1_frame_%2.jpg").arg(baseName).arg(frameNumber));
}