// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef HELPQUERYWG_H
#define HELPQUERYWG_H

#include <QWidget>
#include <QDebug>
#include <QShortcut>

#include "common/common.h"
#include "common/zffprobe.h"
#include "common/ztexthighlighter.h"

#include <widgets/searchwg.h>

namespace Ui {
class HelpQueryWg;
}

#define LONG_FMT "long"           // Print advanced tool options in addition to the basic tool options.
#define FULL_FMT "full"           // Print complete list of options, including shared and private options for encoders, decoders, demuxers, muxers, filters, etc.
#define DECODER_FMT "decoder"     // Print detailed information about the decoder
#define ENCODER_FMT "encoder"     // Print detailed information about the encoder
#define DEMUXER_FMT "demuxer"     // Print detailed information about the demuxer
#define MUXER_FMT "muxer"         // Print detailed information about the muxer
#define FILTER_FMT "filter"       // Print detailed information about the filter
#define BSF_FMT "bsf"             // Print detailed information about the bitstream filter
#define PROTOCOL_FMT "protocol"   // Print detailed information about the protocol

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

    void on_searchReady();
    void on_param_combox_activated(int index);
    void on_keep_last_cbx_stateChanged(int state);

private:
    Ui::HelpQueryWg *ui;
    ZFfprobe m_probe;
    ZTextHighlighter *m_highLighter;
    SearchWG *m_searchWG;
};

#endif // HELPQUERYWG_H
