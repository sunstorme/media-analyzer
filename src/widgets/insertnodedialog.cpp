// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "insertnodedialog.h"
#include "ui_insertnodedialog.h"

#include <QMessageBox>
#include <QDoubleValidator>

InsertNodeDialog::InsertNodeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InsertNodeDialog)
{
    ui->setupUi(this);

    // Setup type group as exclusive
    setupTypeGroup();

    // Initially show value edit for String type
    onTypeChanged();

    // Connect signals
    connect(ui->typeString, &QRadioButton::toggled, this, &InsertNodeDialog::onTypeChanged);
    connect(ui->typeNumber, &QRadioButton::toggled, this, &InsertNodeDialog::onTypeChanged);
    connect(ui->typeBoolean, &QRadioButton::toggled, this, &InsertNodeDialog::onTypeChanged);
    connect(ui->typeNull, &QRadioButton::toggled, this, &InsertNodeDialog::onTypeChanged);
    connect(ui->typeObject, &QRadioButton::toggled, this, &InsertNodeDialog::onTypeChanged);
    connect(ui->typeArray, &QRadioButton::toggled, this, &InsertNodeDialog::onTypeChanged);

    connect(ui->keyEdit, &QLineEdit::textChanged, this, &InsertNodeDialog::updatePreview);
    connect(ui->valueEdit, &QLineEdit::textChanged, this, &InsertNodeDialog::updatePreview);
    connect(ui->boolCombo, &QComboBox::currentTextChanged, this, &InsertNodeDialog::updatePreview);

    // Override accept to add validation
    disconnect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &InsertNodeDialog::onAccepted);

    // Initial preview
    updatePreview();
}

InsertNodeDialog::~InsertNodeDialog()
{
    delete ui;
}

void InsertNodeDialog::setupTypeGroup()
{
    // Type radio buttons are already in a group box, they are auto-exclusive within it
}

void InsertNodeDialog::setKeyEnabled(bool enabled)
{
    ui->keyEdit->setEnabled(enabled);
    ui->keyLabel->setEnabled(enabled);
}

void InsertNodeDialog::setKey(const QString &key)
{
    ui->keyEdit->setText(key);
}

QString InsertNodeDialog::getKey() const
{
    return ui->keyEdit->text();
}

QJsonValue::Type InsertNodeDialog::getSelectedType() const
{
    if (ui->typeString->isChecked()) return QJsonValue::String;
    if (ui->typeNumber->isChecked()) return QJsonValue::Double;
    if (ui->typeBoolean->isChecked()) return QJsonValue::Bool;
    if (ui->typeNull->isChecked()) return QJsonValue::Null;
    if (ui->typeObject->isChecked()) return QJsonValue::Object;
    if (ui->typeArray->isChecked()) return QJsonValue::Array;
    return QJsonValue::String;
}

QVariant InsertNodeDialog::getValue() const
{
    switch (getSelectedType()) {
    case QJsonValue::String:
        return ui->valueEdit->text();
    case QJsonValue::Double:
        return ui->valueEdit->text().toDouble();
    case QJsonValue::Bool:
        return ui->boolCombo->currentText() == "true";
    case QJsonValue::Null:
        return QVariant();
    case QJsonValue::Object:
    case QJsonValue::Array:
        return QVariant();
    default:
        return ui->valueEdit->text();
    }
}

void InsertNodeDialog::setParentIsArray(bool isArray)
{
    m_parentIsArray = isArray;
    if (isArray) {
        setKeyEnabled(false);
        setKey(QString::number(0)); // Placeholder, actual index assigned by model
    }
}

void InsertNodeDialog::onTypeChanged()
{
    QJsonValue::Type type = getSelectedType();

    // Show/hide value inputs based on type
    bool showValueEdit = (type == QJsonValue::String || type == QJsonValue::Double);
    bool showBoolCombo = (type == QJsonValue::Bool);

    ui->valueEdit->setVisible(showValueEdit);
    ui->valueLabel->setVisible(showValueEdit);
    ui->boolCombo->setVisible(showBoolCombo);
    ui->boolLabel->setVisible(showBoolCombo);

    // Set default values based on type
    if (type == QJsonValue::String) {
        ui->valueEdit->setText("new_value");
        ui->valueEdit->setValidator(nullptr);
    } else if (type == QJsonValue::Double) {
        ui->valueEdit->setText("0");
        ui->valueEdit->setValidator(new QDoubleValidator(this));
    }

    updatePreview();
}

void InsertNodeDialog::updatePreview()
{
    QString key = ui->keyEdit->text();
    if (m_parentIsArray) {
        key = "[index]";
    }

    QString preview;
    QJsonValue::Type type = getSelectedType();

    switch (type) {
    case QJsonValue::String:
        preview = QString("\"%1\": \"%2\"").arg(key).arg(ui->valueEdit->text());
        break;
    case QJsonValue::Double:
        preview = QString("\"%1\": %2").arg(key).arg(ui->valueEdit->text());
        break;
    case QJsonValue::Bool:
        preview = QString("\"%1\": %2").arg(key).arg(ui->boolCombo->currentText());
        break;
    case QJsonValue::Null:
        preview = QString("\"%1\": null").arg(key);
        break;
    case QJsonValue::Object:
        preview = QString("\"%1\": {}").arg(key);
        break;
    case QJsonValue::Array:
        preview = QString("\"%1\": []").arg(key);
        break;
    default:
        preview = QString("\"%1\": \"\"").arg(key);
        break;
    }

    ui->previewLabel->setText(preview);
}

void InsertNodeDialog::onAccepted()
{
    // Validate key
    if (!m_parentIsArray && ui->keyEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Key cannot be empty."));
        return;
    }

    // Validate number value
    if (getSelectedType() == QJsonValue::Double) {
        bool ok = false;
        ui->valueEdit->text().toDouble(&ok);
        if (!ok) {
            QMessageBox::warning(this, tr("Validation Error"), tr("Invalid number value."));
            return;
        }
    }

    accept();
}
