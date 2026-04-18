// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "streamswg.h"
#include "ui_streamswg.h"

StreamsWG::StreamsWG(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StreamsWG)
    , m_streamsWidget(nullptr)
{
    ui->setupUi(this);

    m_streamsWidget = new JsonFormatWG(this);
    m_streamsWidget->setControlMargin(0, 0, 0, 0);

    ui->streamsLayout->addWidget(m_streamsWidget);
}

StreamsWG::~StreamsWG()
{
    delete ui;
}

void StreamsWG::setStreamsData(const QByteArray &data)
{
    if (m_streamsWidget) {
        m_streamsWidget->loadData(data);
    }
}

JsonFormatWG* StreamsWG::streamsWidget() const
{
    return m_streamsWidget;
}
