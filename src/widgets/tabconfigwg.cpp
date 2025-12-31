// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "tabconfigwg.h"
#include "ui_tabconfigwg.h"

TabConfigWg::TabConfigWg(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TabConfigWg)
{
    ui->setupUi(this);

    m_configFlowLayout = new ZFlowLayout();
    // ui->config_tb->setSearchTitleVisiable(false);
    ui->config_category_group_layout->insertLayout(0, m_configFlowLayout);
    ui->category_gbx->setChecked(false);
    emit ui->category_gbx->clicked(false);
}

TabConfigWg::~TabConfigWg()
{
    delete ui;
}

void TabConfigWg::setupConfigGroup(const QStringList &configKey)
{
    QStringList tmpconfigKeys = configKey;
    if (!configKey.contains(ALL_DEFAULT_CONFIG_KEY) && configKey.size() > 0) {
        tmpconfigKeys.prepend(ALL_DEFAULT_CONFIG_KEY);
    }

    for (auto it : tmpconfigKeys) {
        QPushButton * btn = new QPushButton(it, this);
        btn->setObjectName(it);
        connect(btn, &QPushButton::clicked, [=](){
            loadConfigData(btn->objectName());
        });
        m_configFlowLayout->addWidget(btn);
    }
}

void TabConfigWg::setupConfigs(const QStringList headers, const QMap<QString, QList<QStringList> > configs)
{
    // title
    m_configs = configs;
    ui->config_tb->initHeaderDetailTb(headers);
    setupConfigGroup(m_configs.keys());

    if (configs.size() > 0) {
        loadConfigData(ALL_DEFAULT_CONFIG_KEY);
    }
}

void TabConfigWg::addContextMenu(QMenu *menu)
{
    ui->config_tb->addContextMenuMenu(menu);
}

void TabConfigWg::addContextAction(QAction *action)
{
    ui->config_tb->addContextMenuAction(action);
}

QList<QStringList> TabConfigWg::getSelectLines()
{
    return ui->config_tb->getSelectLines();
}

void TabConfigWg::loadConfigData(const QString &key)
{
    QList<QStringList> data;

    if (!m_configs.contains(key)) {
        qWarning() << "No match config key in list :" << key;
    } if (key == ALL_DEFAULT_CONFIG_KEY) {
        ui->config_tb->updateDataDetailTb(m_configs, "=");
        return;
    }

    ui->config_tb->updateDataDetailTb(m_configs.value(key), "=");
}

void TabConfigWg::on_category_gbx_clicked(bool checked)
{
    if (!checked) {
        ui->category_gbx->setFixedHeight(18);
    } else {
        ui->category_gbx->setMinimumHeight(0);
        ui->category_gbx->setMaximumHeight(QWIDGETSIZE_MAX);
        ui->category_gbx->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }
}

