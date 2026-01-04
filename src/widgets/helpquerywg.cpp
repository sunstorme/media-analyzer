// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "helpquerywg.h"
#include "ui_helpquerywg.h"

HelpQueryWg::HelpQueryWg(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HelpQueryWg)
{
    ui->setupUi(this);
    ui->category_combx->addItems(HELP_OPTION_FORMATS);

    QShortcut *ctrl_d_shortcut = new QShortcut(QKeySequence("Ctrl+D"), this);
    connect(ctrl_d_shortcut, &QShortcut::activated, this, [this]() {
        ui->control_header_wg->setVisible(!ui->control_header_wg->isVisible());
    });

    emit ui->category_combx->activated(0);
}

HelpQueryWg::~HelpQueryWg()
{
    delete ui;
}

bool HelpQueryWg::setHelpParams(const QString &category, const QString &value)
{
    ui->search_output_ple->clear();

    QStringList helpList {
        QString("%1%2")
            .arg(category).arg(value.isEmpty() ? "" : "=" + value)
    };

    QString helpText = m_probe.getHelp(helpList);
    if (helpText.isEmpty()) {
        helpText = tr("No help information available for %1=%2").arg(category, value);
    }
    
    ui->search_output_ple->setPlainText(helpText);
        
    return !helpText.isEmpty();
}

void HelpQueryWg::setControlHeaderVisiable(const bool &visiable)
{
    ui->control_header_wg->setVisible(visiable);
}

void HelpQueryWg::on_category_combx_activated(int index)
{
    QString currentCategory = ui->category_combx->currentText();
    
    // Show/hide parameter dropdown based on category
    bool showParamBox = !QStringList{"full", "long"}.contains(currentCategory);
    ui->param_combox->setVisible(showParamBox);

    ui->param_combox->clear();
    
    // Query directly if no parameters needed
    if (!showParamBox) {
        setHelpParams(currentCategory, "");
        return;
    }

    // Load corresponding parameters by category
    QStringList items;
    bool success = false;
    
    if (currentCategory == DECODER_FMT) {
        items = m_probe.getCodecsOrMuxersNames(currentCategory);
        success = !items.isEmpty();
    } else if (currentCategory == ENCODER_FMT) {
        items = m_probe.getCodecsOrMuxersNames(currentCategory);
        success = !items.isEmpty();
    } else if (currentCategory == DEMUXER_FMT) {
        items = m_probe.getCodecsOrMuxersNames(currentCategory);
        success = !items.isEmpty();
    } else if (currentCategory == MUXER_FMT) {
        items = m_probe.getCodecsOrMuxersNames(currentCategory);
        success = !items.isEmpty();
    } else if (currentCategory == FILTER_FMT) {
        items = m_probe.getFiltersNames();
        success = !items.isEmpty();
    } else if (currentCategory == BSF_FMT) {
        items = m_probe.getBsfsNames();
        success = !items.isEmpty();
    } else if (currentCategory == PROTOCOL_FMT) {
        items = m_probe.getProtocolNames();
        success = !items.isEmpty();
    }

    if (success && !items.isEmpty()) {
        ui->param_combox->addItems(items);
        // Auto-select first item
        if (items.size() > 0) {
            setHelpParams(currentCategory, items.first());
        }
    } else {
        // Show error when loading fails
        ui->search_output_ple->setPlainText(tr("Failed to load %1 parameters").arg(currentCategory));
    }
}

void HelpQueryWg::on_param_combox_activated(int index)
{
    setHelpParams(ui->category_combx->currentText(), ui->param_combox->currentText());
}

void HelpQueryWg::on_keep_last_cbx_stateChanged(int state)
{
    // Clear content when unchecking keep_last_cbx
    if (state == Qt::Unchecked) {
        ui->search_output_ple->clear();
    }
}
