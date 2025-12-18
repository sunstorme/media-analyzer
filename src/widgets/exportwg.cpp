// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "exportwg.h"
#include "ui_exportwg.h"
#include <QDir>

ExportWG::ExportWG(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ExportWG)
{
    ui->setupUi(this);

    QDateTime currentTime = QDateTime::currentDateTime();

    // Initialize the flow layout for search range
    m_mediaInfoFloatLayout = new ZFlowLayout();
    ui->mediainfo_fileds_gbox->setLayout(m_mediaInfoFloatLayout);

    m_basicInfoFloatLayout = new ZFlowLayout();
    ui->baseinfo_fileds_gbox->setLayout(m_basicInfoFloatLayout);

    // Create select all and select none checkboxes
    m_selectAllMediaInfoRBtn = new QRadioButton(tr("Select All"), this);
    m_selectNoneMediaInfoRBtn = new QRadioButton(tr("Select None"), this);

    m_selectAllBasicInfoRBtn = new QRadioButton(tr("Select All"), this);
    m_selectNoneBasicInfoRBtn = new QRadioButton(tr("Select None"), this);

    // Style the control checkboxes differently
    m_selectAllMediaInfoRBtn->setObjectName("selectAllMediaInfoRbtn");
    m_selectNoneMediaInfoRBtn->setObjectName("selectNoneMediaInfoRBtn");

    m_selectAllBasicInfoRBtn->setObjectName("selectAllBasicInfoRBtn");
    m_selectNoneBasicInfoRBtn->setObjectName("selectNoneBasicInfoRBtn");

    // Add them to the layout first
    m_mediaInfoFloatLayout->addWidget(m_selectAllMediaInfoRBtn);
    m_mediaInfoFloatLayout->addWidget(m_selectNoneMediaInfoRBtn);

    m_basicInfoFloatLayout->addWidget(m_selectAllBasicInfoRBtn);
    m_basicInfoFloatLayout->addWidget(m_selectNoneBasicInfoRBtn);

    // Connect signals
    connect(m_selectAllMediaInfoRBtn, &QCheckBox::clicked, this, &ExportWG::onMediaInfoSelectAllClicked);
    connect(m_selectNoneMediaInfoRBtn, &QCheckBox::clicked, this, &ExportWG::onMediaInfoSelectNoneClicked);

    connect(m_selectAllBasicInfoRBtn, &QCheckBox::clicked, this, &ExportWG::onBasicInfoSelectAllClicked);
    connect(m_selectNoneBasicInfoRBtn, &QCheckBox::clicked, this, &ExportWG::onBasicInfoSelectNoneClicked);

    // Setup context menu for export button
    ui->export_btn->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->export_btn, &QPushButton::customContextMenuRequested,
            this, &ExportWG::showExportButtonContextMenu);

    ui->save_dir_le->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->save_dir_le, &QPushButton::customContextMenuRequested,
            this, &ExportWG::showExportButtonContextMenu);
}

ExportWG::~ExportWG()
{
    delete ui;
}

void ExportWG::setExportModel(ExportModelType type)
{
    m_exportModel = type;

    ui->baseinfo_fileds_gbox->setVisible(false);
    ui->mediainfo_fileds_gbox->setVisible(false);
    ui->media_format_gbox->setVisible(false);

    if (m_exportModel & BasicInfo) {
        ui->baseinfo_fileds_gbox->setVisible(true);
    }

    if (m_exportModel & MediaInfo) {
        ui->mediainfo_fileds_gbox->setVisible(true);
        ui->media_format_gbox->setVisible(true);
    }
}

void ExportWG::setMediaInfoExportFiledsOptions(const QStringList &options)
{
    // Clear existing checkboxes (except select all/none)
    clearMediaInfoExportFiledsOptions();

    // Create new checkboxes for each option
    for (const QString &option : options) {
        auto *checkBox = new QCheckBox(option, this);
        checkBox->setChecked(true);
        m_exportMediaInfoFiledsCBoxes.append(checkBox);
        m_mediaInfoFloatLayout->addWidget(checkBox);

        // Connect to the toggle signal
        connect(checkBox, &QCheckBox::toggled, this, &ExportWG::onMediaInfoSearchRangeRBtnToggled);
    }

    // Update the layout
    m_selectAllMediaInfoRBtn->setChecked(true);
    m_mediaInfoFloatLayout->update();
}

void ExportWG::clearMediaInfoExportFiledsOptions()
{
    // Remove and delete all search range checkboxes
    for (auto *checkBox : m_exportMediaInfoFiledsCBoxes) {
        m_mediaInfoFloatLayout->removeWidget(checkBox);
        checkBox->deleteLater();
    }
    m_exportMediaInfoFiledsCBoxes.clear();
}

QStringList ExportWG::getMediaInfoSelectedExportFileds() const
{
    QStringList selectedOptions;
    for (const auto *checkBox : m_exportMediaInfoFiledsCBoxes) {
        if (checkBox->isChecked()) {
            selectedOptions.append(checkBox->text());
        }
    }
    return selectedOptions;
}

void ExportWG::setMediaInfoSelectedExportFileds(const QStringList &selectedOptions)
{
    for (auto *checkBox : m_exportMediaInfoFiledsCBoxes) {
        checkBox->setChecked(selectedOptions.contains(checkBox->text()));
    }

    // Update select all checkbox state
    bool allSelected = !m_exportMediaInfoFiledsCBoxes.isEmpty() &&
                       m_exportMediaInfoFiledsCBoxes.size() == getMediaInfoSelectedExportFileds().size();
    m_selectAllMediaInfoRBtn->setChecked(allSelected);

    emit exportMediaInfoFiledsSelectionChanged(getMediaInfoSelectedExportFileds());
}

void ExportWG::setBasicInfoExportFiledsOptions(const QStringList &options)
{
    // Clear existing checkboxes (except select all/none)
    clearBasicInfoExportFiledsOptions();

    // Create new checkboxes for each option
    for (const QString &option : options) {
        auto *checkBox = new QCheckBox(option, this);
        checkBox->setChecked(true);
        m_exportBasicInfoFiledsCBoxes.append(checkBox);
        m_basicInfoFloatLayout->addWidget(checkBox);

        // Connect to the toggle signal
        connect(checkBox, &QCheckBox::toggled, this, &ExportWG::onBasicInfoSearchRangeRBtnToggled);
    }

    // Update the layout
    m_selectAllBasicInfoRBtn->setChecked(true);
    m_basicInfoFloatLayout->update();
}

void ExportWG::clearBasicInfoExportFiledsOptions()
{
    // Remove and delete all search range checkboxes
    for (auto *checkBox : m_exportBasicInfoFiledsCBoxes) {
        m_basicInfoFloatLayout->removeWidget(checkBox);
        checkBox->deleteLater();
    }
    m_exportBasicInfoFiledsCBoxes.clear();
}

QStringList ExportWG::getBasicInfoSelectedExportFileds() const
{
    QStringList selectedOptions;
    for (const auto *checkBox : m_exportBasicInfoFiledsCBoxes) {
        if (checkBox->isChecked()) {
            selectedOptions.append(checkBox->text());
        }
    }
    return selectedOptions;
}

void ExportWG::setBasicInfoSelectedExportFileds(const QStringList &selectedOptions)
{
    for (auto *checkBox : m_exportBasicInfoFiledsCBoxes) {
        checkBox->setChecked(selectedOptions.contains(checkBox->text()));
    }

    // Update select all checkbox state
    bool allSelected = !m_exportBasicInfoFiledsCBoxes.isEmpty() &&
                       m_exportBasicInfoFiledsCBoxes.size() == getBasicInfoSelectedExportFileds().size();
    m_selectAllBasicInfoRBtn->setChecked(allSelected);

}

void ExportWG::setInputMediaFilePath(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    if (fileInfo.exists()) {
        m_input_fileName = fileName;
        m_save_name = fileInfo.fileName() + "_media_info";
        m_save_dir = fileInfo.absolutePath();
        ui->save_filename_le->setText(m_save_name);
        ui->save_dir_le->setText(fileInfo.absolutePath());
    } else {
        if (m_save_name.isEmpty()) {
            m_save_name = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
            ui->save_filename_le->setText(m_save_name);
        }
        if (m_save_dir.isEmpty()) {
            m_save_dir = QDir::currentPath();
            ui->save_dir_le->setText(m_save_dir);
        }


        qWarning() << fileName << tr("not exists! will use default:") << m_save_dir << m_save_name;
    }
}

void ExportWG::onMediaInfoSelectAllClicked(bool checked)
{
    // Block signals temporarily to avoid recursive calls
    const bool blocked = blockSignals(true);

    for (auto *checkBox : m_exportMediaInfoFiledsCBoxes) {
        checkBox->setChecked(checked);
    }

    blockSignals(blocked);
    emit exportMediaInfoFiledsSelectionChanged(getMediaInfoSelectedExportFileds());
}

void ExportWG::onMediaInfoSelectNoneClicked()
{
    // Block signals temporarily to avoid recursive calls
    const bool blocked = blockSignals(true);

    for (auto *checkBox : m_exportMediaInfoFiledsCBoxes) {
        checkBox->setChecked(false);
    }

    blockSignals(blocked);
    emit exportMediaInfoFiledsSelectionChanged(getMediaInfoSelectedExportFileds());
}

void ExportWG::onMediaInfoSearchRangeRBtnToggled()
{
    // Update select all/none states based on current selection
    const QStringList selected = getMediaInfoSelectedExportFileds();
    const bool allSelected = !m_exportMediaInfoFiledsCBoxes.isEmpty() &&
                             selected.size() == m_exportMediaInfoFiledsCBoxes.size();
    const bool noneSelected = selected.isEmpty();

    // Block signals to avoid recursive calls
    const bool blocked = blockSignals(true);

    if (!allSelected && !allSelected) {
        m_selectAllMediaInfoRBtn->setAutoExclusive(false);
        m_selectNoneMediaInfoRBtn->setAutoExclusive(false);

        m_selectAllMediaInfoRBtn->setChecked(false);
        m_selectNoneMediaInfoRBtn->setChecked(false);
    } else {
        m_selectAllMediaInfoRBtn->setAutoExclusive(true);
        m_selectNoneMediaInfoRBtn->setAutoExclusive(true);
        m_selectAllMediaInfoRBtn->setChecked(allSelected);
        m_selectNoneMediaInfoRBtn->setChecked(noneSelected);
    }

    blockSignals(blocked);
    emit exportMediaInfoFiledsSelectionChanged(selected);
}

void ExportWG::onBasicInfoSelectAllClicked(bool checked)
{
    const bool blocked = blockSignals(true);

    for (auto *checkBox : m_exportBasicInfoFiledsCBoxes) {
        checkBox->setChecked(checked);
    }

    blockSignals(blocked);
    emit exportBasicInfoFiledsSelectionChanged(getMediaInfoSelectedExportFileds());
}

void ExportWG::onBasicInfoSelectNoneClicked()
{
    const bool blocked = blockSignals(true);

    for (auto *checkBox : m_exportBasicInfoFiledsCBoxes) {
        checkBox->setChecked(false);
    }

    blockSignals(blocked);
    emit exportBasicInfoFiledsSelectionChanged(getMediaInfoSelectedExportFileds());
}

void ExportWG::onBasicInfoSearchRangeRBtnToggled()
{
    // Update select all/none states based on current selection
    const QStringList selected = getBasicInfoSelectedExportFileds();
    const bool allSelected = !m_exportBasicInfoFiledsCBoxes.isEmpty() &&
                             selected.size() == m_exportBasicInfoFiledsCBoxes.size();
    const bool noneSelected = selected.isEmpty();

    // Block signals to avoid recursive calls
    const bool blocked = blockSignals(true);

    if (!allSelected && !allSelected) {
        m_selectAllBasicInfoRBtn->setAutoExclusive(false);
        m_selectNoneBasicInfoRBtn->setAutoExclusive(false);

        m_selectAllBasicInfoRBtn->setChecked(false);
        m_selectNoneBasicInfoRBtn->setChecked(false);
    } else {
        m_selectAllBasicInfoRBtn->setAutoExclusive(true);
        m_selectNoneBasicInfoRBtn->setAutoExclusive(true);
        m_selectAllBasicInfoRBtn->setChecked(allSelected);
        m_selectNoneBasicInfoRBtn->setChecked(noneSelected);
    }

    blockSignals(blocked);
    emit exportBasicInfoFiledsSelectionChanged(selected);
}

void ExportWG::on_save_filename_le_textChanged(const QString &arg1)
{
    m_save_name = arg1;
}


void ExportWG::on_export_btn_clicked()
{
    QStringList cmds;

    if (m_exportModel & BasicInfo) {
        // 0. basic info
        for (auto it : m_exportBasicInfoFiledsCBoxes) {
            QString basicInfo_Cmd = QString("%1 %2 %3 %4 > %5")
            .arg(FFPROBE)
                .arg(LOGLEVEL)
                .arg(QUIET)
                .arg(it->text())
                .arg(QDir(m_save_dir).filePath(QString("%1%2.txt").arg(FFPROBE).arg(it->text())));

            cmds << basicInfo_Cmd;
        }
    }

    if (m_exportModel & MediaInfo) {
        // 1. default
        if (ui->default_cbox->isChecked()) {
            QString deault_Cmd = QString("%1 %2 %3 %4 -i %5 -of default=nk=%6:nw=%7 > %8")
            .arg(FFPROBE)
                .arg(LOGLEVEL)
                .arg(QUIET)
                .arg(getMediaInfoSelectedExportFileds().join(" "))
                .arg(m_input_fileName)
                .arg(ui->default_nokey_cbox->isChecked() ? "1" : "0")
                .arg(ui->default_noprint_wrappers_cbox->isChecked() ? "1" : "0")
                .arg(QDir(m_save_dir).filePath(m_save_name + "_default_" + ui->default_suffix_le->text().trimmed()));

            cmds << deault_Cmd;
        }

        // 2. json
        if (ui->json_cbox->isChecked()) {
            QString json_Cmd = QString("%1 %2 %3 %4 -i %5 -of json=c=%6 > %7")
            .arg(FFPROBE)
                .arg(LOGLEVEL)
                .arg(QUIET)
                .arg(getMediaInfoSelectedExportFileds().join(" "))
                .arg(m_input_fileName)
                .arg(ui->json_compact_cbox->isChecked() ? "1" : "0")
                .arg(QDir(m_save_dir).filePath(m_save_name + "_json_" + ui->json_suffix_le->text().trimmed()));

            cmds << json_Cmd;
        }

        // 3. ini
        if (ui->ini_cbox->isChecked()) {
            QString ini_Cmd = QString("%1 %2 %3 %4 -i %5 -of ini=h=%6 > %7")
            .arg(FFPROBE)
                .arg(LOGLEVEL)
                .arg(QUIET)
                .arg(getMediaInfoSelectedExportFileds().join(" "))
                .arg(m_input_fileName)
                .arg(ui->ini_hierarchical_cbox->isChecked() ? "1" : "0")
                .arg(QDir(m_save_dir).filePath(m_save_name + "_ini_"  + ui->ini_suffix_le->text().trimmed()));

            cmds << ini_Cmd;
        }

        // 4. xml
        if (ui->xml_cbox->isChecked()) {
            QString xml_Cmd = QString("%1 %2 %3 %4 -i %5 -of xml=q=%6:x=%7 > %8")
            .arg(FFPROBE)
                .arg(LOGLEVEL)
                .arg(QUIET)
                .arg(getMediaInfoSelectedExportFileds().join(" "))
                .arg(m_input_fileName)
                .arg(ui->xml_fully_qualified_cbox->isChecked() ? "1" : "0")
                .arg(ui->xml_xsd_strict_cbox->isChecked() ? "1" : "0")
                .arg(QDir(m_save_dir).filePath(m_save_name  + "_xml_" + ui->xml_suffix_le->text().trimmed()));

            cmds << xml_Cmd;
        }

        // 5. flat
        if (ui->flat_cbox->isChecked()) {
            QString flat_Cmd = QString("%1 %2 %3 %4 -i %5 -of flat=s=%6:h=%7 > %8")
            .arg(FFPROBE)
                .arg(LOGLEVEL)
                .arg(QUIET)
                .arg(getMediaInfoSelectedExportFileds().join(" "))
                .arg(m_input_fileName)
                .arg(ui->flat_sep_char_le->text().trimmed())
                .arg(ui->flat_hierarchical_cbox->isChecked() ? "1" : "0")
                .arg(QDir(m_save_dir).filePath(m_save_name + "_flat_"  + ui->flat_suffix_le->text().trimmed()));

            cmds << flat_Cmd;
        }

        // 6. compact, csv
        if (ui->compact_cbox->isChecked()) {
            QString compact_Cmd = QString("%1 %2 %3 %4 -i %5 -of compact=s=%6:nk=%7:e=%8:p=%9 > %10")
            .arg(FFPROBE)
                .arg(LOGLEVEL)
                .arg(QUIET)
                .arg(getMediaInfoSelectedExportFileds().join(" "))
                .arg(m_input_fileName)
                .arg(ui->compact_item_sep_le->text().trimmed())
                .arg(ui->compact_nokey_cbox->isChecked() ? "1" : "0")
                .arg(ui->compact_escape_combox->currentText().trimmed())
                .arg(ui->compact_print_section_cbox->isChecked() ? "1" : "0")
                .arg(QDir(m_save_dir).filePath(m_save_name + "_csv_"  + ui->compact_suffix_le->text().trimmed()));

            cmds << compact_Cmd;
        }
    }

    // log out
    for (auto it : cmds) {
        qDebug() << "export cmd: " << it;
    }

    ProgressDialog *progressDlg = new ProgressDialog;
    progressDlg->setWindowTitle(tr("Export Files"));
    progressDlg->setProgressMode(ProgressDialog::Determinate);
    progressDlg->setMessage("Export...");
    progressDlg->setAutoClose(true);

    progressDlg->start();

    if (m_executor) {
        delete m_executor;
        m_executor = nullptr;
    }

    m_executor = new ZCommandExecutor(this);

    connect(m_executor, &ZCommandExecutor::progressUpdated,
            [=](int completed, int total, const QString &message){
                emit progressDlg->rangeChanged(1, total);
                emit progressDlg->valueChanged(completed);
                emit progressDlg->messageChanged(message);
            });

    connect(m_executor, &ZCommandExecutor::commandStarted,
            [=](const QString &command, int index){
                emit progressDlg->messageChanged(tr("Started: %1 %2").arg(index).arg(command));
            });

    connect(m_executor, &ZCommandExecutor::commandFinished,
            [=](const QString &command, int index, int exitCode, QProcess::ExitStatus exitStatus){
                QString status = (exitCode == 0 && exitStatus == QProcess::NormalExit) ? "Success" : "Failed";
                qDebug() << index << command << status;

                if (ui->preview_cbox->isChecked()) {
                    QStringList cmdList = command.split(">", QT_SKIP_EMPTY_PARTS);
                    if (cmdList.size() == 2) {
                        QString fileName = cmdList.at(1).trimmed();
                        QFile file(fileName);
                        if (file.exists()) {
                            QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
                        }
                    }
                }
            });

    connect(m_executor, &ZCommandExecutor::allCommandsFinished,
            [=](bool success){
                if (success) {
                    qDebug() << tr("All commands completed successfully");
                } else {
                    qDebug() << tr("Some commands failed or were stopped");
                }

                QDir dir(ui->save_dir_le->text());
                if (dir.exists()) {
                    QDesktopServices::openUrl(QUrl::fromLocalFile(ui->save_dir_le->text()));
                }

                if (success) {
                    emit progressDlg->messageChanged("Finsh parse");
                    progressDlg->finish();
                    progressDlg->deleteLater();
                }
            });

    connect(m_executor, &ZCommandExecutor::commandOutput,
            [=](const QString &command, const QString &output, int index)
            {
                qDebug() << tr("Output from index %1 %2: %3").arg(index).arg(command).arg(output.trimmed());
            });

    connect(m_executor, &ZCommandExecutor::commandError,
            [=](const QString &command, const QString &error, int index)
            {
                qDebug() << tr("Error from index %1 %2: %3").arg(index).arg(command).arg(error.trimmed());
            });

    m_executor->executeCommands(cmds);
    progressDlg->exec();
}

void ExportWG::onSelectSaveDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("Select Save Directory"),
                                                    m_save_dir.isEmpty() ? QDir::homePath() : m_save_dir,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        m_save_dir = dir;
        ui->save_dir_le->setText(dir);
    }
}

void ExportWG::onOpenSaveDirectory()
{
    QString dir = m_save_dir.isEmpty() ? QDir::homePath() : m_save_dir;
    QDir checkDir(dir);

    if (!checkDir.exists()) {
        // If directory doesn't exist, try to create it
        if (!checkDir.mkpath(dir)) {
            qWarning() << tr("Cannot create directory: %1").arg(dir);
            return;
        }
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void ExportWG::showExportButtonContextMenu(const QPoint &pos)
{
    QMenu *contextMenu = new QMenu(this);
    contextMenu->setAttribute(Qt::WA_DeleteOnClose);

    QAction *selectDirAction = contextMenu->addAction(tr("Select Save Directory"));
    QAction *openDirAction = contextMenu->addAction(tr("Open Save Directory"));

    connect(selectDirAction, &QAction::triggered, this, &ExportWG::onSelectSaveDirectory);
    connect(openDirAction, &QAction::triggered, this, &ExportWG::onOpenSaveDirectory);

    QWidget *widget = static_cast<QWidget *>(QObject::sender()) ;
    if (widget) {
        contextMenu->exec(widget->mapToGlobal(pos));
    }
}
