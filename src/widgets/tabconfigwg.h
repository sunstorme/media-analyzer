// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef TABCONFIGWG_H
#define TABCONFIGWG_H

#include <QWidget>
#include <QDebug>
#include <QPushButton>
#include <QSizePolicy>

#include <common/zflowlayout.h>

#define ALL_DEFAULT_CONFIG_KEY "All"

namespace Ui {
class TabConfigWg;
}

class TabConfigWg : public QWidget
{
    Q_OBJECT

public:
    explicit TabConfigWg(QWidget *parent = nullptr);
    ~TabConfigWg();

    void setupConfigGroup(const QStringList &configKey);
    void setupConfigs(const QStringList headers, const QMap<QString, QList<QStringList>>configs);

    void addContextMenu(QMenu *menu);

    void addContextAction(QAction *action);

    QList <QStringList> getSelectLines();
private slots:
    void on_category_gbx_clicked(bool checked);

private:
    void loadConfigData(const QString& key);

private:
    Ui::TabConfigWg *ui;

    ZFlowLayout *m_configFlowLayout = nullptr;

    QMap<QString, QList<QStringList>> m_configs;
};

#endif // TABCONFIGWG_H
