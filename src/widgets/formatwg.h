// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef FORMATWG_H
#define FORMATWG_H

#include <QWidget>

#include <common/zsingleton.h>
#include <widgets/jsonfmtwg.h>

namespace Ui {
class FormatWG;
}

class FormatWG : public QWidget
{
    Q_OBJECT

public:
    DECLARE_ZSINGLETON(FormatWG)
    FormatWG(QWidget *parent = nullptr);
    ~FormatWG();

public:
    void setFormatData(const QByteArray &data);
    JsonFormatWG* formatWidget() const;

private:
    Ui::FormatWG *ui;
    JsonFormatWG *m_formatWidget = nullptr;
};

#endif // FORMATWG_H
