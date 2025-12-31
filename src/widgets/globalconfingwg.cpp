// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "globalconfingwg.h"
#include "ui_globalconfingwg.h"

GlobalConfingWG::GlobalConfingWG(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GlobalConfingWG)
{
    ui->setupUi(this);
    generalCfgWg = new TableFormatWG(this);

    setWindowTitle("Global Config");
    setupButtonGroup();
    
    ui->genreral_tab_layout->addWidget(generalCfgWg);

    connect(generalCfgWg, &TableFormatWG::dataChanged, [=](QStringList configs) {
        if (configs.size() == 2) {
            QStringList settingParts = configs.at(0).split("/", QT_SKIP_EMPTY_PARTS);
            if (settingParts.size() == 2) {
                QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
                settings.beginGroup(settingParts.at(0));
                settings.setValue(settingParts.at(1), configs.at(1));
                settings.endGroup();
            }
        }
    });
}

GlobalConfingWG::~GlobalConfingWG()
{
    delete ui;
}

void GlobalConfingWG::setCurrentConfig(const QString &group)
{
    loadConfigData(group);
}

void GlobalConfingWG::setupButtonGroup()
{
    configButtonGroup = new QButtonGroup(this);
    configButtonGroup->setExclusive(true);
    for (auto it : CONFIG_GROUPS) {
        QPushButton * btn = new QPushButton(it, this);
        connect(btn, &QPushButton::clicked, [=](){
            loadConfigData(btn->text());
        });
        ui->config_category_group->addWidget(btn);
    }

}


void GlobalConfingWG::loadConfigData(const QString& group, const QStringList& keys)
{
    QStringList headers{"Key", "Value"};
    QList<QStringList> data;

    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    
    if (!group.isEmpty()) {
        settings.beginGroup(group);
    }

    QStringList keysToLoad = keys;
    if (keysToLoad.isEmpty()) {
        keysToLoad = settings.allKeys();
    }

    for (const QString &key : keysToLoad) {
        if (settings.contains(key)) {
            QVariant value = settings.value(key);
            QString displayKey = group.isEmpty() ? key : group + "/" + key;
            data << QStringList{displayKey, value.toString()};
        }
    }

    if (!group.isEmpty()) {
        settings.endGroup();
    }

    generalCfgWg->initHeaderDetailTb(headers);
    generalCfgWg->updateDataDetailTb(data, "=");

}
