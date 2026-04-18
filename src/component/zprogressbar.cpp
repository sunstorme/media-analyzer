// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zprogressbar.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QApplication>
#include <QStyleOption>

ZProgressBar::ZProgressBar(QWidget *parent)
    : QWidget(parent)
{
    initColors();

    m_busyTimer = new QTimer(this);
    connect(m_busyTimer, &QTimer::timeout, this, &ZProgressBar::updateBusyAnimation);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(24);
}

void ZProgressBar::initColors()
{
    // Use theme-aware colors
    m_progressColor = QColor(0x2d, 0xc6, 0x5e);  // Green
    m_backgroundColor = QColor(0xe0, 0xe0, 0xe0); // Light gray
}

void ZProgressBar::setRange(int minimum, int maximum)
{
    m_minimum = minimum;
    m_maximum = maximum;

    if (isBusy()) {
        m_value = -1;
        m_busyTimer->start(20);
    } else {
        m_busyTimer->stop();
        m_busyPosition = 0.0;
        if (m_value < m_minimum) m_value = m_minimum;
        if (m_value > m_maximum) m_value = m_maximum;
    }

    update();
}

void ZProgressBar::setMinimum(int minimum)
{
    setRange(minimum, m_maximum);
}

void ZProgressBar::setMaximum(int maximum)
{
    setRange(m_minimum, maximum);
}

int ZProgressBar::minimum() const
{
    return m_minimum;
}

int ZProgressBar::maximum() const
{
    return m_maximum;
}

int ZProgressBar::value() const
{
    return m_value;
}

void ZProgressBar::setValue(int value)
{
    if (isBusy()) return;

    m_value = value;
    if (m_value < m_minimum) m_value = m_minimum;
    if (m_value > m_maximum) m_value = m_maximum;
    update();
}

void ZProgressBar::reset()
{
    m_value = m_minimum;
    m_busyPosition = 0.0;
    m_busyDirection = 1;
    update();
}

void ZProgressBar::setTextVisible(bool visible)
{
    m_textVisible = visible;
    update();
}

bool ZProgressBar::isTextVisible() const
{
    return m_textVisible;
}

void ZProgressBar::setProgressColor(const QColor &color)
{
    m_progressColor = color;
    update();
}

QColor ZProgressBar::progressColor() const
{
    return m_progressColor;
}

void ZProgressBar::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    update();
}

QColor ZProgressBar::backgroundColor() const
{
    return m_backgroundColor;
}

bool ZProgressBar::isBusy() const
{
    return m_minimum == 0 && m_maximum == 0;
}

QSize ZProgressBar::sizeHint() const
{
    return QSize(200, 24);
}

QSize ZProgressBar::minimumSizeHint() const
{
    return QSize(40, 24);
}

void ZProgressBar::updateBusyAnimation()
{
    // Move the busy indicator back and forth
    m_busyPosition += 0.02 * m_busyDirection;

    if (m_busyPosition >= 1.0) {
        m_busyPosition = 1.0;
        m_busyDirection = -1;
    } else if (m_busyPosition <= 0.0) {
        m_busyPosition = 0.0;
        m_busyDirection = 1;
    }

    update();
}

QString ZProgressBar::formatText() const
{
    if (isBusy()) return QString();

    if (m_maximum == 0 || m_maximum == m_minimum) return QString();

    int percentage = (m_value - m_minimum) * 100 / (m_maximum - m_minimum);
    return QString("%1%").arg(percentage);
}

void ZProgressBar::paintEvent(QPaintEvent */*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QStyleOption opt;
    opt.initFrom(this);

    int w = width();
    int h = height();
    qreal radius = h / 2.0;

    // Draw background (track)
    QPainterPath bgPath;
    bgPath.addRoundedRect(0, 0, w, h, radius, radius);
    painter.fillPath(bgPath, m_backgroundColor);

    if (isBusy()) {
        // Busy mode: draw a moving bar
        qreal barWidth = w * 0.3; // Bar takes 30% of width
        qreal maxPos = w - barWidth;
        qreal x = m_busyPosition * maxPos;

        QPainterPath barPath;
        barPath.addRoundedRect(x, 0, barWidth, h, radius, radius);

        QLinearGradient gradient(x, 0, x + barWidth, 0);
        QColor startColor = m_progressColor.lighter(120);
        QColor endColor = m_progressColor;
        gradient.setColorAt(0.0, startColor);
        gradient.setColorAt(1.0, endColor);

        painter.setClipPath(bgPath);
        painter.fillPath(barPath, gradient);
    } else if (m_maximum > m_minimum) {
        // Determinate mode: draw progress fill
        qreal progress = qreal(m_value - m_minimum) / qreal(m_maximum - m_minimum);
        qreal fillWidth = w * progress;

        if (fillWidth > 0) {
            QPainterPath fillPath;
            fillPath.addRoundedRect(0, 0, fillWidth, h, radius, radius);

            QLinearGradient gradient(0, 0, 0, h);
            QColor topColor = m_progressColor.lighter(110);
            QColor bottomColor = m_progressColor;
            gradient.setColorAt(0.0, topColor);
            gradient.setColorAt(1.0, bottomColor);

            painter.setClipPath(bgPath);
            painter.fillPath(fillPath, gradient);
        }

        // Draw text
        if (m_textVisible) {
            painter.setClipping(false);
            QString text = formatText();
            painter.setPen(Qt::black);
            QFont font = painter.font();
            font.setPixelSize(h * 0.6);
            painter.setFont(font);
            painter.drawText(QRect(0, 0, w, h), Qt::AlignCenter, text);
        }
    }
}
