// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef GLOBALCONFINGWG_H
#define GLOBALCONFINGWG_H

#include <QWidget>
#include <QSettings>
#include <QCoreApplication>
#include <QDebug>
#include <QButtonGroup>
#include <QPushButton>

#include <widgets/tablefmtwg.h>

#include <common/qtcompat.h>

namespace Ui {
class GlobalConfingWG;
}

class GlobalConfingWG : public QWidget
{
    Q_OBJECT

public:
    explicit GlobalConfingWG(QWidget *parent = nullptr);
    ~GlobalConfingWG();

    void setCurrentConfig(const QString& group);

private:
    Ui::GlobalConfingWG *ui;

    TableFormatWG *generalCfgWg = nullptr;
    QButtonGroup *configButtonGroup = nullptr;

private:
    void loadConfigData(const QString& group, const QStringList& keys = QStringList());
    void setupButtonGroup();
};

#endif // GLOBALCONFINGWG_H
