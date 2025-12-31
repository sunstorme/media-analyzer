// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef PROGRESSDLG_H
#define PROGRESSDLG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    enum ProgressMode {
        Determinate,    // Determinate progress (with max value)
        Indeterminate,  // Indeterminate progress (looping animation)
        Busy            // Busy state (no progress bar, only animation)
    };

    explicit ProgressDialog(QWidget *parent = nullptr,
                            Qt::WindowFlags flags = Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    ~ProgressDialog();

    // Set progress mode
    void setProgressMode(ProgressMode mode);

    // Set window title
    void setWindowTitle(const QString &title);

    // Set message text
    void setMessage(const QString &message);

    // Set progress value (for Determinate mode)
    void setValue(int value);
    void setRange(int minimum, int maximum);

    // Show/hide cancel button
    void setCancelButtonVisible(bool visible);

    // Enable auto close
    void setAutoClose(bool autoClose);

    // Check if canceled
    bool wasCanceled() const;

    // Reset progress
    void reset();

public slots:
    // Start progress (for Indeterminate/Busy modes)
    void start();

    // Finish progress
    void finish();

    // Cancel progress
    void cancel();

signals:
    // Cancel signal
    void canceled();

    // Progress update signals
    void rangeChanged(int minimum, int maximum);
    void valueChanged(int value);
    void messageChanged(const QString &message);

    void toFinish();
protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void updateAnimation();

private:
    void setupUI();
    void updateProgressStyle();

    QProgressBar *m_progressBar = nullptr;
    QLabel *m_messageLabel = nullptr;
    QLabel *m_timeLabel = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QTimer *m_animationTimer = nullptr;
    QElapsedTimer *m_elapsedTimer = nullptr;

    ProgressMode m_mode;
    bool m_autoClose;
    bool m_canceled;
    int m_animationStep;

    QString formatTime(qint64 milliseconds);
};

#endif // PROGRESSDLG_H
