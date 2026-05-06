// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef CONFIGUREBUILDWIDGET_H
#define CONFIGUREBUILDWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QProcess>
#include <QDateTime>
#include <QTextCursor>

#include <common/qtcompat.h>

namespace Ui {
class ConfigureBuildWidget;
}

class ZConfigureConfig;

class ConfigureBuildWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigureBuildWidget(QWidget *parent = nullptr);
    ~ConfigureBuildWidget();

    QMap<QString, QList<QStringList> > getOptions();

    QString getConfigCmd();
private slots:
    void on_show_cmd_cbx_toggled(bool checked);

    void on_select_local_path_btn_clicked();

    void on_build_btn_clicked();

    void on_configure_btn_clicked();

    void on_install_btn_clicked();

    void on_clean_btn_clicked();

    void on_local_projects_combx_currentIndexChanged(int index);

private:
    void loadSettings();
    void saveSettings();
    void parseOptionLineFast(const QString &line, QString &option, QString &description, QString &defaultValue);

    void loadConfigureOptions();
private:
    Ui::ConfigureBuildWidget *ui;
    QAction *m_addRecordAction = nullptr;
    QAction *m_RemoveRecordAction = nullptr;

    QStringList m_filePaths;
    QProcess *m_process = nullptr;
    ZConfigureConfig *m_config = nullptr;

    QMap<QString, QStringList> m_optionMaps;
};

#endif // CONFIGUREBUILDWIDGET_H
