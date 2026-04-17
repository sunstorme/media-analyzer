// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "configurebuildtool.h"
#include "ui_configurebuildtool.h"

ConfigureBuildTool::ConfigureBuildTool(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ConfigureBuildTool)
{
    ui->setupUi(this);

    loadSettings();

    for (const QString &path : m_filePaths) {
        if (!path.isEmpty() && QDir(path).exists()) {
            ui->local_projects_combx->addItem(path);
        }
    }

    if (ui->local_projects_combx->count() > 0) {
        ui->local_projects_combx->setCurrentIndex(0);

        loadConfigureOptions();
    }

    m_addRecordAction = new QAction("Add Too Selected Options");
    m_RemoveRecordAction = new QAction("Remove Selected");

    ui->configure_complie_options_tb->addContextAction(m_addRecordAction);
    ui->select_option_tb->addContextMenuAction(m_RemoveRecordAction);

    // action connect
    connect(m_addRecordAction, &QAction::triggered, [=]() {
        ui->select_option_tb->clearDetailTb();
        ui->select_option_tb->initHeaderDetailTb(QStringList{"Option", "Description", "Default Value"});
        ui->select_option_tb->appendDataDetailTb(ui->configure_complie_options_tb->getSelectLines());

        ui->cmd_ple->setPlainText(getConfigCmd());
    });

    connect(m_RemoveRecordAction, &QAction::triggered, [=]() {
        ui->select_option_tb->removeSelectedRow();

        ui->cmd_ple->setPlainText(getConfigCmd());
    });

    connect(ui->select_option_tb, &TableFormatWG::dataChanged, [=](){
        ui->cmd_ple->setPlainText(getConfigCmd());
    });

    emit ui->show_cmd_cbx->toggled(false);
    ui->select_option_tb->setSearchTitleVisiable(false);

    m_process = new QProcess(this);

    ui->complie_output_ple->clear();

    QString startTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->complie_output_ple->appendHtml(
        QString("<font color='blue'>=== Progress start at: %1 ===</font>").arg(startTime)
        );

    connect(m_process, &QProcess::readyReadStandardOutput, this, [=]() {
        QByteArray output = m_process->readAllStandardOutput();
        QString text = QString::fromLocal8Bit(output);

        QString timestamp = QTime::currentTime().toString("hh:mm:ss");
        ui->complie_output_ple->appendPlainText("[" + timestamp + "] " + text.trimmed());

    });

    connect(m_process, &QProcess::readyReadStandardError, this, [=]() {
        QByteArray error = m_process->readAllStandardError();
        QString text = QString::fromLocal8Bit(error);

        QString timestamp = QTime::currentTime().toString("hh:mm:ss");
        ui->complie_output_ple->appendHtml(
            "<font color='red'>[" + timestamp + "] " + text+ text.trimmed() + "</font>"
            );

    });

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
                QString endTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                QString color = (exitCode == 0) ? "green" : "red";
                QString status = (exitStatus == QProcess::NormalExit) ? "Normal exit" : "Crush exit";

                ui->build_btn->setEnabled(true);

                ui->complie_output_ple->appendHtml(
                    QString("<font color='%1'>=== Progress end at: %2 ===</font>").arg(color).arg(endTime)
                    );
                ui->complie_output_ple->appendHtml(
                    QString("<font color='%1'>Exit code: %2, statue: %3</font>").arg(color).arg(exitCode).arg(status)
                    );
            });
}

ConfigureBuildTool::~ConfigureBuildTool()
{
    delete ui;
}

QMap<QString, QList<QStringList>> ConfigureBuildTool::getOptions()
{
    QMap<QString, QList<QStringList>> options;

    QProcess process;
    QString optionText;

    process.setWorkingDirectory(ui->local_projects_combx->currentText());
    process.start("bash", QStringList() << "./configure" << "-h");

    if (process.waitForFinished(30000)) {
        QByteArray output = process.readAllStandardOutput();
        QByteArray error = process.readAllStandardError();

        if (!output.isEmpty()) {
            optionText = output;
        } else if (!error.isEmpty()) {
            qDebug() << "Error:" << QString::fromLocal8Bit(error);
        }
    } else {
        qDebug() << "Process timeout or error:" << process.errorString();
    }

    QStringList optionsLines = optionText.split("\n", QT_SKIP_EMPTY_PARTS);

    QString option, description, defaultValue, lastkey;
    for (auto line: optionsLines) {
        if (line.contains("Usage:") || line.contains("Options:") || line.contains("NOTE:")) {
            continue;
        }

        line = line.trimmed();
        if (line.endsWith(":")) {
            lastkey = line.replace("options:", "").trimmed();
            options.insert(lastkey.replace(":", ""), QList<QStringList>{});
            continue;
        }

        if (line.startsWith("--")) {
            defaultValue.clear();
            parseOptionLineFast(line, option, description, defaultValue);
            options[lastkey].append(QStringList{option, description, defaultValue});
            continue;
        } else {
            if (options[lastkey].size() != 0) {
                QString combineOption = options[lastkey].last().join(" ").append(" " + line.trimmed());
                defaultValue.clear();
                parseOptionLineFast(combineOption, option, description, defaultValue);
                options[lastkey].last() = QStringList{option, description, defaultValue};
            } else {
                continue;
            }
        }
    }

    // for (const QString& category : options.keys()) {
    //     qDebug() << "=== " << category << " ===";
    //     for (auto it : options[category]) {
    //         qDebug() << it.join(" ");
    //     }
    // }

    return options;
}

QString ConfigureBuildTool::getConfigCmd()
{
    QString cmd = "./configure \\\n";

    for (auto it: *ui->select_option_tb->getTableData()) {
        QStringList optionList = it.at(0).split("=", QT_SKIP_EMPTY_PARTS);

        QString value = it.at(2);

        cmd.append(QString("%1%2%3 \\\n")
                       .arg(optionList.at(0))
                       .arg(value.trimmed().isEmpty() ? "" : "=")
                       .arg(value));
    }

    cmd = cmd.trimmed();
    cmd.remove(cmd.length() - 1, 1);
    cmd = cmd.trimmed();
    return cmd;
}

void ConfigureBuildTool::on_show_cmd_cbx_toggled(bool checked)
{
    ui->cmd_ple->setVisible(checked);
}

void ConfigureBuildTool::on_select_local_path_btn_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        tr("Select configure source folder"),
        m_filePaths.isEmpty() ? QDir::homePath() : m_filePaths.first()
        );

    if (folderPath.isEmpty())
        return;

    folderPath = QDir::toNativeSeparators(folderPath);
    if (!QDir(folderPath).exists()) {
        qWarning() << "Selected folder not exist!";
        return;
    }

    m_filePaths.removeAll(folderPath);
    m_filePaths.prepend(folderPath);

    const int maxHistory = 10;
    if (m_filePaths.size() > maxHistory) {
        m_filePaths = m_filePaths.mid(0, maxHistory);
    }

    QString currentText = ui->local_projects_combx->currentText();

    ui->local_projects_combx->clear();
    ui->local_projects_combx->addItems(m_filePaths);

    int index = ui->local_projects_combx->findText(currentText);
    if (index != -1) {
        ui->local_projects_combx->setCurrentIndex(index);
    } else if (ui->local_projects_combx->count() > 0) {
        ui->local_projects_combx->setCurrentIndex(0);
    }

    loadConfigureOptions();
    saveSettings();
}

void ConfigureBuildTool::loadSettings()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    settings.beginGroup(CONFIGURE_BUILDER_SETTINGS_GROUP);
    m_filePaths = settings.value(CONFIGURE_BUILDER_RECENTFOLDERS_KEY).toStringList();
    settings.endGroup();
}

void ConfigureBuildTool::saveSettings()
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    settings.beginGroup(CONFIGURE_BUILDER_SETTINGS_GROUP);
    settings.setValue(CONFIGURE_BUILDER_RECENTFOLDERS_KEY, m_filePaths);
    settings.endGroup();

    settings.sync();
}

void ConfigureBuildTool::on_build_btn_clicked()
{
    m_process->setWorkingDirectory(ui->local_projects_combx->currentText());
    if (m_process->workingDirectory() != ui->local_projects_combx->currentText()) {
        m_process->start("cd", QStringList{ ui->local_projects_combx->currentText()});
    }
    qDebug() << m_process->workingDirectory();

    QString cmd = "make";
    QStringList arguments;
    arguments << "-j" << QString::number(std::thread::hardware_concurrency());

    for (auto &it : arguments) {
        it = it.trimmed();
    }

    if (arguments.isEmpty()) return;
    qDebug() << cmd << arguments;
    m_process->start(cmd, arguments);
}

void ConfigureBuildTool::on_configure_btn_clicked()
{
    ui->build_btn->setEnabled(false);

    m_process->setWorkingDirectory(ui->local_projects_combx->currentText());
    if (m_process->workingDirectory() != ui->local_projects_combx->currentText()) {
        m_process->start("cd", QStringList{ ui->local_projects_combx->currentText()});
    }
    qDebug() << m_process->workingDirectory();

    QString cmd = "bash";
    QString command = ui->cmd_ple->toPlainText().trimmed();
    QStringList arguments = command.split("\\", QT_SKIP_EMPTY_PARTS);

    for (auto &it : arguments) {
        it = it.trimmed();
    }

    if (arguments.isEmpty()) return;
    qDebug() << cmd << arguments;
    m_process->start(cmd, arguments);
}

void ConfigureBuildTool::on_install_btn_clicked()
{
    m_process->setWorkingDirectory(ui->local_projects_combx->currentText());
    if (m_process->workingDirectory() != ui->local_projects_combx->currentText()) {
        m_process->start("cd", QStringList{ ui->local_projects_combx->currentText()});
    }
    qDebug() << m_process->workingDirectory();

    QString cmd = "make";
    QStringList arguments;
    arguments << "install";

    for (auto &it : arguments) {
        it = it.trimmed();
    }

    if (arguments.isEmpty()) return;
    qDebug() << cmd << arguments;
    m_process->start(cmd, arguments);
}

void ConfigureBuildTool::on_clean_btn_clicked()
{
    m_process->kill();

    m_process->setWorkingDirectory(ui->local_projects_combx->currentText());
    if (m_process->workingDirectory() != ui->local_projects_combx->currentText()) {
        m_process->start("cd", QStringList{ ui->local_projects_combx->currentText()});
    }
    qDebug() << m_process->workingDirectory();

    QString cmd = "make";
    QStringList arguments;
    arguments << "clean";

    for (auto &it : arguments) {
        it = it.trimmed();
    }

    if (arguments.isEmpty()) return;
    qDebug() << cmd << arguments;
    m_process->start(cmd, arguments);
}

void ConfigureBuildTool::parseOptionLineFast(const QString& line, QString& option, QString& description, QString& defaultValue) {
    option.clear();
    description.clear();
    defaultValue.clear();

    int firstSpace = line.indexOf(' ');
    if (firstSpace == -1) {
        option = line.trimmed();
        return;
    }

    option = line.left(firstSpace).trimmed();

    int lastBracket = line.lastIndexOf('[');
    if (lastBracket > firstSpace) {
        description = line.mid(firstSpace + 1, lastBracket - firstSpace - 1).trimmed();
        defaultValue = line.mid(lastBracket).trimmed().remove("[").remove("]");
    } else {
        description = line.mid(firstSpace + 1).trimmed();
    }
}

void ConfigureBuildTool::loadConfigureOptions()
{
    auto config = getOptions();
    ui->configure_complie_options_tb->setupConfigs(QStringList{"Option", "Description", "Default Value"}, config);
}


void ConfigureBuildTool::on_local_projects_combx_currentIndexChanged(int index)
{
    if (index >= 0) {
        loadConfigureOptions();
    }
}

