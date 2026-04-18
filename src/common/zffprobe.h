// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZFFPROBE_H
#define ZFFPROBE_H

#include <QObject>
#include <QProcess>
#include <QMap>
#include <QVariantMap>
#include <QColor>
#include <QRgb>

#define FFPROBE "ffprobe"
#define VERSION "-version"
#define BUILDCONF "-buildconf"
#define FORMATS "-formats"
#define MUXERS "-muxers"
#define DEMUXERS "-demuxers"
#define DEVICES "-devices"
#define CODECS "-codecs"
#define DECODERS "-decoders"
#define ENCODERS "-encoders"
#define BSFS "-bsfs"
#define PROTOCOLS "-protocols"
#define FILTERS "-filters"
#define PIX_FMTS "-pix_fmts"
#define LAYOUTS "-layouts"
#define SAMPLE_FMTS "-sample_fmts"
#define COLORS "-colors"
#define HIDEBANNER "-hide_banner"
#define LICENSE "-L"
#define HELP "-help"

// -show_format
#define SHOW_FORMAT "-show_format"       // show format/container info
#define FORMAT "format"
#define FILENAME "filename"
#define NB_STREAMS "nb_streams"
#define NB_PROGRAMS "nb_programs"
#define FORMAT_NAME "format_name"
#define FORMAT_LONG_NAME "format_long_name"
#define START_TIME "start_time"
#define DURATION "duration"
#define SIZE "size"
#define BIT_RATE "bit_rate"
#define PROBE_SCORE "probe_score"

#define TAGS "tags"
#define MAJOR_BRAND "major_brand"
#define MINOR_VERSION "minor_version"
#define COMPATIBLE_BRANDS "compatible_brands"
#define CREATION_TIME "creation_time"
#define TITLE "title"
#define ARTIST "artist"
#define ENCODER "encoder"
#define COPYRIGHT "copyright"
#define DESCRIPTION "description"

// log
#define LOGLEVEL "-loglevel"             // set logging level
#define FV "-v"                           // set logging level
// log flags
#define REPEAT "repeat"                  // Indicates that repeated log output should not be compressed to the first line and the "Last message repeated n times" line will be omitted.
#define LEVEL "level"                    // Indicates that log output should add a [level] prefix to each message line. This can be used as an alternative to log coloring, e.g. when dumping the log to file.
#define TIME "time"                      // Indicates that log lines should be prefixed with time information.
#define DATETIME "datetime"              // Indicates that log lines should be prefixed with date and time information.
// -loglevel
#define QUIET "quiet"                    // Show nothing at all; be silent.
#define PANIC "panic"                    // Only show fatal errors which could lead the process to crash, such as an assertion failure. This is not currently used for anything.
#define FATAL "fatal"                    // Only show fatal errors. These are errors after which the process absolutely cannot continue.
#define ERROR "error"                    // Show all errors, including ones which can be recovered from.
#define WARNING "warning"                // Show all warnings and errors. Any message related to possibly incorrect or unexpected events will be shown.
#define INFO "info"                      // Show informative messages during processing. This is in addition to warnings and errors. This is the default value.
#define VERBOSE "verbose"                // Same as info, except more verbose.
#define DEBUG "debug"                    // Show everything, including debugging information.
#define TRACE "trace"                    // shows extremely detailed, low-level tracing information about the internal execution flow of the code, allowing for deep technical debugging and analysis.

// log led
#define SHOW_ERROR "-show_error"         // show probing error
#define SHOW_LOG "-show_log"             // show log

#define REPORT "-report"                 // generate a report
#define MAX_ALLOC "-max_alloc"           // set maximum size of a single allocated block
#define CPUFLAGS "-cpuflags"             // force specific cpu flags
#define HIDE_BANNER "-hide_banner"       // do not show program banner
#define SOURCES "-sources"               // list sources of the input device
#define SINKS "-sinks"                   // list sinks of the output device
#define Ff "-f"                           // force format
#define UNIT "-unit"                     // show unit of the displayed values
#define PREFIX "-prefix"                 // use SI prefixes for the displayed values
#define BYTE_BINARY_PREFIX "-byte_binary_prefix" // use binary prefixes for byte units
#define SEXAGESIMAL "-sexagesimal"       // use sexagesimal format HOURS:MM:SS.MICROSECONDS for time units
#define PRETTY "-pretty"                 // prettify the format of displayed values, make it more human readable

#define SELECT_STREAMS "-select_streams" // select the specified streams
#define SECTIONS "-sections"             // print sections structure and section information, and exit
#define SHOW_DATA "-show_data"           // show packets data
#define SHOW_DATA_HASH "-show_data_hash" // show packets data hash

#define SHOW_FRAMES "-show_frames"       // show frames info
#define SHOW_FORMAT_ENTRY "-show_format_entry" // show a particular entry from the format/container info
#define SHOW_ENTRIES "-show_entries"     // show a set of specified entries
#define SHOW_PACKETS "-show_packets"     // show packets info
#define SHOW_PROGRAMS "-show_programs"   // show programs info
#define SHOW_STREAMS "-show_streams"     // show streams info
#define SHOW_CHAPTERS "-show_chapters"   // show chapters info
#define COUNT_FRAMES "-count_frames"     // count the number of frames per stream
#define COUNT_PACKETS "-count_packets"   // count the number of packets per stream

// version
#define SHOW_PROGRAM_VERSION "-show_program_version" // show ffprobe version
#define SHOW_LIBRARY_VERSIONS "-show_library_versions" // show library versions
#define SHOW_VERSIONS "-show_versions"   // show program and library versions
#define SHOW_PIXEL_FORMATS "-show_pixel_formats" // show pixel format descriptions

#define SHOW_PRIVATE_DATA "-show_private_data" // show private data
#define PRIVATE "-private"               // same as show_private_data
#define BITEXACT "-bitexact"             // force bitexact output
#define READ_INTERVALS "-read_intervals" // set read intervals
#define DEFAULT "-default"               // generic catch all option
#define FI "-i"                           // read specified file
#define PRINT_FILENAME "-print_filename" // override the printed input filename
#define FIND_STREAM_INFO "-find_stream_info" // read and decode the streams to fill missing information with heuristi

// -print_format
#define PRINT_FORMAT "-print_format"
#define OF "-of"
#define JSON "json"
#define XML "xml"
#define INI "ini"
#define FLAT "flat"
#define CSV "csv"

// program
#define SHOW_FRAMES_VIDEO "-show_frames_video"       // show frames info of video
#define SHOW_FRAMES_AUDIO "-show_frames_audio"       // show frames info of audio
#define SHOW_PACKETS_VIDEO "-show_packets_video"       // show packets info of video
#define SHOW_PACKETS_AUDIO "-show_packets_audio"       // show packets info of audio

#define GET "get"
#define V4L2 "v4l2"
#define ALSA "alsa"

#define LONG_FMT "long"           // Print advanced tool options in addition to the basic tool options.
#define FULL_FMT "full"           // Print complete list of options, including shared and private options for encoders, decoders, demuxers, muxers, filters, etc.
#define DECODER_FMT "decoder"     // Print detailed information about the decoder
#define ENCODER_FMT "encoder"     // Print detailed information about the encoder
#define DEMUXER_FMT "demuxer"     // Print detailed information about the demuxer
#define MUXER_FMT "muxer"         // Print detailed information about the muxer
#define FILTER_FMT "filter"       // Print detailed information about the filter
#define BSF_FMT "bsf"             // Print detailed information about the bitstream filter
#define PROTOCOL_FMT "protocol"   // Print detailed information about the protocol

enum CodecType {
    CODEC_TYPE_DECODER = 1,
    CODEC_TYPE_ENCODER = 2
};

static QStringList ffmpegCommandList = {HIDEBANNER, LOGLEVEL, QUIET};

class ZFfprobe : public QObject
{
    Q_OBJECT
public:
    explicit ZFfprobe(QObject *parent = nullptr);

/*
 * Basic Info
 */
    Q_INVOKABLE QString getVersion();               // show version
    Q_INVOKABLE QString getBuildconf();             // show build configuration
    Q_INVOKABLE QString getFormats();               // show available formats
    Q_INVOKABLE QString getMuxers();                // show available muxers
    Q_INVOKABLE QString getDemuxers();              // show available demuxers
    Q_INVOKABLE QString getDevices();               // show available devices
    Q_INVOKABLE QString getCodecs();                // show available codecs
    Q_INVOKABLE QString getDecoders();              // show available decoders
    Q_INVOKABLE QString getEncoders();              // show available encoders
    Q_INVOKABLE QString getBsfs();                  // show available bit stream filters
    Q_INVOKABLE QString getProtocols();             // show available protocols
    Q_INVOKABLE QString getFilters();               // show available filters
    Q_INVOKABLE QString getPixfmts();               // show available pixel formats
    Q_INVOKABLE QString getLayouts();               // show standard channel layouts
    Q_INVOKABLE QString getSamplefmts ();           // show available audio sample formats
    Q_INVOKABLE QString getColors();                // show available color names
    Q_INVOKABLE QString getL();                     // show license.
    Q_INVOKABLE QString getVideoSize();             // show video size.
    Q_INVOKABLE QString getVideoRate();             // show video rate
    Q_INVOKABLE QString getSources();               // show Sources alsa, v4l2
    Q_INVOKABLE QString getSinks();                 // show Sinks alsa, v4l2

    Q_INVOKABLE QString getHelp(const QStringList& helpList);      // show help info.
    Q_INVOKABLE QString getBasicInfo(const QString& function, bool *sucess = nullptr);

/*
 * Media Info
 */
    Q_INVOKABLE QString getMediaInfoJsonFormat(const QString& command, const QString& fileName);           // show format/container info
    
    // Packet/Frame count methods
    int getPacketCount(const QString& fileName, int streamIndex = 0);
    int getFrameCount(const QString& fileName, int streamIndex = 0);

    // Stream info methods
    struct StreamInfo {
        int index;
        QString codecType;
        QString codecName;
        QString language;
        QString title;
    };
    
    QList<StreamInfo> getMediaStreams(const QString& fileName);

    QStringList getCodecsOrMuxersNames(const QString& key);
    QStringList getFiltersNames();
    QStringList getBsfsNames();
    QStringList getProtocolNames();

/*
 * Utilities Info
 */
    Q_INVOKABLE QMap<QString, QList<QVariant>> getVideoSizeMap(const QString &key = "");             // get video size.
    Q_INVOKABLE QMap<QString, QList<QVariant>> getVideoRateMap(const QString &key = "");             // get video rate.
    Q_INVOKABLE QMap<QString, QList<QVariant>> getColor(const QString &key);                         // get color.
signals:

private:
    QString getFFprobeCommandOutput(const QString& command, const QStringList &otherParms = {});
private:
    QString cacheVersion;
};

#endif // ZFFPROBE_H
