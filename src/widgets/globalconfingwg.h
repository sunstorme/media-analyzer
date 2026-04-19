// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef GLOBALCONFINGWG_H
#define GLOBALCONFINGWG_H

#include <QWidget>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>

#include <widgets/jsonfmtwg.h>
#include <common/zjsonconfig.h>

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

    void setCurrentConfig(const QString &group);

private:
    Ui::GlobalConfingWG *ui;

    JsonFormatWG *m_jsonView = nullptr;
    QButtonGroup *configButtonGroup = nullptr;
    QLabel *m_statusLabel = nullptr;
    QString m_currentGroup;

private:
    void setupButtonGroup();
    void setupJsonView();
    void setupStatusBar();
    void loadConfigGroup(const QString &group);
    void saveCurrentGroup();
    void onConfigChanged(const QString &path);
    void onExternalConfigChanged();
    void showStatusMessage(const QString &message, int timeout = 3000);
};

#endif // GLOBALCONFINGWG_H
