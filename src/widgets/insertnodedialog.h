// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef INSERTNODEDIALOG_H
#define INSERTNODEDIALOG_H

#include <QDialog>
#include <QJsonValue>

namespace Ui {
class InsertNodeDialog;
}

class InsertNodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsertNodeDialog(QWidget *parent = nullptr);
    ~InsertNodeDialog();

    void setKeyEnabled(bool enabled);
    void setKey(const QString &key);
    QString getKey() const;

    QJsonValue::Type getSelectedType() const;
    QVariant getValue() const;

    void setParentIsArray(bool isArray);

private slots:
    void onTypeChanged();
    void updatePreview();
    void onAccepted();

private:
    Ui::InsertNodeDialog *ui;
    bool m_parentIsArray = false;

    void setupTypeGroup();
};

#endif // INSERTNODEDIALOG_H
