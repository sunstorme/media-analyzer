// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZFFPLAY_H
#define ZFFPLAY_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QTemporaryFile>

class ZFFplay : public QObject
{
    Q_OBJECT

public:
    explicit ZFFplay(QObject *parent = nullptr);
    ~ZFFplay();

    // Play video file with ffplay
    bool playVideo(const QString &inputFile);
    
    // Play video file with specific options
    bool playVideoWithOptions(const QString &inputFile, const QStringList &options);
    
    // Display image file without autoexit
    bool displayImage(const QString &imageFile);
    
    // Display image file with window size limits
    bool displayImageWithSize(const QString &imageFile, int maxWidth = 800, int maxHeight = 600, int frameNumber = -1, const QString &fileName = QString());
    
    // Stop playback
    void stop();
    
    // Check if ffplay is available
    static bool isAvailable();
    
    // Get current playback state
    bool isPlaying() const;

signals:
    void playbackStarted();
    void playbackFinished(int exitCode);
    void errorOccurred(const QString &error);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    QProcess *m_process = nullptr;
    bool m_isPlaying;
    
    // Execute ffplay command
    bool executeCommand(const QStringList &arguments);
};

#endif // ZFFPLAY_H
