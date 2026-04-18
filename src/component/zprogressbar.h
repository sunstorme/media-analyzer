// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZPROGRESSBAR_H
#define ZPROGRESSBAR_H

#include <QWidget>
#include <QTimer>

class ZProgressBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(bool textVisible READ isTextVisible WRITE setTextVisible)
    Q_PROPERTY(QColor progressColor READ progressColor WRITE setProgressColor)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)

public:
    explicit ZProgressBar(QWidget *parent = nullptr);

    void setRange(int minimum, int maximum);
    void setMinimum(int minimum);
    void setMaximum(int maximum);
    int minimum() const;
    int maximum() const;

    int value() const;
    void setValue(int value);
    void reset();

    void setTextVisible(bool visible);
    bool isTextVisible() const;

    void setProgressColor(const QColor &color);
    QColor progressColor() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    // Busy mode: when minimum == maximum == 0
    bool isBusy() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateBusyAnimation();

private:
    int m_minimum = 0;
    int m_maximum = 100;
    int m_value = 0;
    bool m_textVisible = true;

    QColor m_progressColor;
    QColor m_backgroundColor;

    // Busy mode animation
    QTimer *m_busyTimer = nullptr;
    qreal m_busyPosition = 0.0; // 0.0 to 1.0
    int m_busyDirection = 1;    // 1 or -1

    void initColors();
    QString formatText() const;
};

#endif // ZPROGRESSBAR_H
