// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef HELPQUERYWG_H
#define HELPQUERYWG_H

#include <QWidget>
#include <QDebug>
#include <QShortcut>

#include "common/common.h"
#include "common/zffprobe.h"

namespace Ui {
class HelpQueryWg;
}

static const QStringList HELP_OPTION_FORMATS = {
    LONG_FMT,
    FULL_FMT,
    DECODER_FMT,
    ENCODER_FMT,
    DEMUXER_FMT,
    MUXER_FMT,
    FILTER_FMT,
    BSF_FMT,
    PROTOCOL_FMT
};

class HelpQueryWg : public QWidget
{
    Q_OBJECT

public:
    explicit HelpQueryWg(QWidget *parent = nullptr);
    ~HelpQueryWg();

    bool setHelpParams(const QString &category, const QString &value);
    void setControlHeaderVisiable(const bool &visiable = true);

private slots:
    void on_category_combx_activated(int index);
    void on_param_combox_activated(int index);
    void on_keep_last_cbx_stateChanged(int state);

private:
    Ui::HelpQueryWg *ui;
    ZFfprobe m_probe;
};

#endif // HELPQUERYWG_H
