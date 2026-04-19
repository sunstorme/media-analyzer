// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "globalconfingwg.h"
#include "ui_globalconfingwg.h"
#include "common/common.h"

#include <QTimer>
#include <QMessageBox>

GlobalConfingWG::GlobalConfingWG(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GlobalConfingWG)
{
    ui->setupUi(this);

    setWindowTitle(tr("Global Config"));

    setupJsonView();
    setupButtonGroup();
    setupStatusBar();

    // Select the first config group by default
    if (!CONFIG_GROUPS.isEmpty()) {
        setCurrentConfig(CONFIG_GROUPS.first());
    }

    // Connect to ZJsonConfig signals for external change detection
    connect(ZJsonConfig::instance(), &ZJsonConfig::externalConfigChanged,
            this, &GlobalConfingWG::onExternalConfigChanged);
    connect(ZJsonConfig::instance(), &ZJsonConfig::validationError,
            this, [this](const QString &error) {
                showStatusMessage(tr("Validation error: %1").arg(error), 5000);
            });
}

GlobalConfingWG::~GlobalConfingWG()
{
    delete ui;
}

void GlobalConfingWG::setCurrentConfig(const QString &group)
{
    // Check the corresponding radio button
    QList<QAbstractButton *> buttons = configButtonGroup->buttons();
    for (QAbstractButton *btn : buttons) {
        if (btn->objectName() == group) {
            btn->setChecked(true);
            break;
        }
    }
    loadConfigGroup(group);
}

void GlobalConfingWG::setupButtonGroup()
{
    configButtonGroup = new QButtonGroup(this);
    configButtonGroup->setExclusive(true);

    for (auto it : CONFIG_GROUPS) {
        QRadioButton *btn = new QRadioButton(translatedConfigGroupName(it), this);
        btn->setObjectName(it);
        connect(btn, &QRadioButton::clicked, [=]() {
            loadConfigGroup(btn->objectName());
        });
        ui->config_category_group->addWidget(btn);
        configButtonGroup->addButton(btn);
    }
}

void GlobalConfingWG::setupJsonView()
{
    m_jsonView = new JsonFormatWG(this);
    ui->genreral_tab_layout->addWidget(m_jsonView);
}

void GlobalConfingWG::setupStatusBar()
{
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("QLabel { color: gray; padding: 2px; }");
    ui->genreral_tab_layout->addWidget(m_statusLabel);
}

void GlobalConfingWG::loadConfigGroup(const QString &group)
{
    m_currentGroup = group;

    // Set the config save path so edits are saved to the correct group
    m_jsonView->setConfigSavePath(group);

    QJsonObject groupData = ZJsonConfig::instance()->getGroup(group);
    QJsonDocument doc(groupData);

    m_jsonView->loadData(doc.toJson(QJsonDocument::Indented));

    showStatusMessage(tr("Loaded config: %1").arg(translatedConfigGroupName(group)));
}

void GlobalConfingWG::saveCurrentGroup()
{
    if (m_currentGroup.isEmpty()) {
        return;
    }

    // Get the current JSON data from the view
    QString allData = m_jsonView->getData();
    if (allData.isEmpty()) {
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(allData.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        showStatusMessage(tr("JSON parse error: %1").arg(parseError.errorString()), 5000);
        return;
    }

    QJsonObject groupData = doc.object();

    // Validate before saving
    if (!ZJsonConfig::instance()->validateGroup(m_currentGroup, groupData)) {
        showStatusMessage(tr("Validation failed for group: %1").arg(m_currentGroup), 5000);
        return;
    }

    ZJsonConfig::instance()->setGroup(m_currentGroup, groupData);
    showStatusMessage(tr("Saved config: %1").arg(translatedConfigGroupName(m_currentGroup)));
}

void GlobalConfingWG::onConfigChanged(const QString &path)
{
    Q_UNUSED(path);
    // Real-time save is handled by ZJsonConfig::setValue
    // This is for additional UI updates if needed
}

void GlobalConfingWG::onExternalConfigChanged()
{
    // Reload current group if it was changed externally
    if (!m_currentGroup.isEmpty()) {
        showStatusMessage(tr("External config change detected, reloading..."));
        loadConfigGroup(m_currentGroup);
    }
}

void GlobalConfingWG::showStatusMessage(const QString &message, int timeout)
{
    m_statusLabel->setText(message);
    if (timeout > 0) {
        QTimer::singleShot(timeout, [this]() {
            m_statusLabel->clear();
        });
    }
}
