// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "formatwg.h"
#include "ui_formatwg.h"

FormatWG::FormatWG(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormatWG)
    , m_formatWidget(nullptr)
{
    ui->setupUi(this);

    m_formatWidget = new JsonFormatWG(this);
    m_formatWidget->setControlMargin(0, 0, 0, 0);

    ui->formatLayout->addWidget(m_formatWidget);
}

FormatWG::~FormatWG()
{
    delete ui;
}

void FormatWG::setFormatData(const QByteArray &data)
{
    if (m_formatWidget) {
        m_formatWidget->loadData(data);
    }
}

JsonFormatWG* FormatWG::formatWidget() const
{
    return m_formatWidget;
}
