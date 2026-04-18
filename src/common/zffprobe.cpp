// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "zffprobe.h"
#include "qtcompat.h"
#include "qdebug.h"
#include "qrgb.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>

ZFfprobe::ZFfprobe(QObject *parent)
    : QObject{parent}
{}

QString ZFfprobe::getVersion()
{
    if (cacheVersion.isEmpty()) {
        cacheVersion = getFFprobeCommandOutput(VERSION);
    }

    return cacheVersion;
}

QString ZFfprobe::getBuildconf()
{
    return getFFprobeCommandOutput(BUILDCONF);
}

QString ZFfprobe::getFormats()
{
    return getFFprobeCommandOutput(FORMATS);
}

QString ZFfprobe::getMuxers()
{
    return getFFprobeCommandOutput(MUXERS);
}

QString ZFfprobe::getDemuxers()
{
    return getFFprobeCommandOutput(DEMUXERS);
}

QString ZFfprobe::getDevices()
{
    return getFFprobeCommandOutput(DEVICES);
}

QString ZFfprobe::getCodecs()
{
    return getFFprobeCommandOutput(CODECS);
}

QString ZFfprobe::getDecoders()
{
    return getFFprobeCommandOutput(DECODERS);
}

QString ZFfprobe::getEncoders()
{
    return getFFprobeCommandOutput(ENCODERS);
}

QString ZFfprobe::getBsfs()
{
    return getFFprobeCommandOutput(BSFS);
}

QString ZFfprobe::getProtocols()
{
    return getFFprobeCommandOutput(PROTOCOLS);
}

QString ZFfprobe::getFilters()
{
    return getFFprobeCommandOutput(FILTERS);
}

QString ZFfprobe::getPixfmts()
{
    return getFFprobeCommandOutput(PIX_FMTS);
}

QString ZFfprobe::getLayouts()
{
    return getFFprobeCommandOutput(LAYOUTS);
}

QString ZFfprobe::getSamplefmts()
{
    return getFFprobeCommandOutput(SAMPLE_FMTS);
}

QString ZFfprobe::getColors()
{
    return getFFprobeCommandOutput(COLORS);
}

QString ZFfprobe::getL()
{
    return getFFprobeCommandOutput(LICENSE);
}

QString ZFfprobe::getHelp(const QStringList& helpList)
{
    return getFFprobeCommandOutput(HELP, helpList);
}

QString ZFfprobe::getVideoSize()
{
    QString videoSize;

    QMapIterator<QString, QList<QVariant>> i(getVideoSizeMap());
    while (i.hasNext()) {
        i.next();
        videoSize.append(i.key() + " " + i.value().at(0).toString() + "\n");
    }

    return videoSize;
}

QString ZFfprobe::getVideoRate()
{
    QString videoRate;

    QMapIterator<QString, QList<QVariant>> i(getVideoRateMap());
    while (i.hasNext()) {
        i.next();
        videoRate.append(i.key() + " " + i.value().at(0).toString() + "\n");
    }

    return videoRate;
}

QString ZFfprobe::getSources()
{
    QString sources;
    sources.append(getFFprobeCommandOutput(SOURCES, QStringList{V4L2}));
    sources.append(getFFprobeCommandOutput(SOURCES, QStringList{ALSA}));
    return sources;
}

QString ZFfprobe::getSinks()
{
    QString sinks;
    sinks.append(getFFprobeCommandOutput(SINKS, QStringList{V4L2}));
    sinks.append(getFFprobeCommandOutput(SINKS, QStringList{ALSA}));
    return sinks;
}

QString ZFfprobe::getBasicInfo(const QString &function, bool *sucess)
{
    QString retVal;

    QString funWrapper = function;
    if (!funWrapper.startsWith(GET)) {
        funWrapper.replace(0, 1, function[0].toUpper());
        funWrapper.prepend(GET);
    }

    bool tmp_sucess = QMetaObject::invokeMethod(this, funWrapper.toUtf8(), Qt::DirectConnection,
                                                Q_RETURN_ARG(QString, retVal));
    if (sucess) {
        *sucess = tmp_sucess;
    }

    return retVal;
}

QString ZFfprobe::getMediaInfoJsonFormat(const QString& command, const QString& fileName)
{
    QProcess process;
    process.start(FFPROBE, QStringList() << HIDEBANNER <<
                               LOGLEVEL << QUIET <<
                               OF << JSON <<
                               command.split(" ", QT_SKIP_EMPTY_PARTS) <<
                               FI  << fileName);

    qDebug() << process.arguments().join(" ").prepend(" ").prepend(FFPROBE);
    process.waitForFinished(-1);
    return process.readAll();
}

int ZFfprobe::getPacketCount(const QString& fileName, int streamIndex)
{
    if (fileName.isEmpty()) {
        return -1;
    }

    // Fast approach: read nb_frames from stream metadata (no decoding needed).
    // For packets, nb_frames is a reasonable approximation and returns instantly.
    QProcess process;
    process.start(FFPROBE, QStringList()
                  << HIDEBANNER << LOGLEVEL << QUIET
                  << SELECT_STREAMS << QString::number(streamIndex)
                  << SHOW_ENTRIES << "stream=nb_frames"
                  << OF << JSON
                  << FI << fileName);

    qDebug() << process.arguments().join(" ").prepend(" ").prepend(FFPROBE);
    process.waitForFinished(-1);
    QByteArray output = process.readAll();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(output, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Failed to parse packet count JSON:" << error.errorString();
        return -1;
    }

    QJsonObject rootObj = doc.object();
    QJsonArray streamsArray = rootObj.value("streams").toArray();
    if (streamsArray.isEmpty()) {
        return -1;
    }

    QJsonObject streamObj = streamsArray.first().toObject();
    // ffprobe returns nb_frames as a string in JSON (e.g., "24000"), not a number.
    // Must use toString() + QString::toInt() instead of QJsonValue::toInt().
    bool ok = false;
    int count = streamObj.value("nb_frames").toString().toInt(&ok);
    if (!ok || count <= 0) {
        qWarning() << "nb_frames not available in stream metadata, cannot estimate packet count";
        return -1;
    }
    return count;
}

int ZFfprobe::getFrameCount(const QString& fileName, int streamIndex)
{
    if (fileName.isEmpty()) {
        return -1;
    }

    // Fast approach: read nb_frames from stream metadata (no decoding needed).
    // This reads the container header directly and returns instantly, unlike
    // the old -count_frames approach which required decoding the entire file.
    QProcess process;
    process.start(FFPROBE, QStringList()
                  << HIDEBANNER << LOGLEVEL << QUIET
                  << SELECT_STREAMS << QString::number(streamIndex)
                  << SHOW_ENTRIES << "stream=nb_frames"
                  << OF << JSON
                  << FI << fileName);

    qDebug() << process.arguments().join(" ").prepend(" ").prepend(FFPROBE);
    process.waitForFinished(-1);
    QByteArray output = process.readAll();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(output, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Failed to parse frame count JSON:" << error.errorString();
        return -1;
    }

    QJsonObject rootObj = doc.object();
    QJsonArray streamsArray = rootObj.value("streams").toArray();
    if (streamsArray.isEmpty()) {
        return -1;
    }

    QJsonObject streamObj = streamsArray.first().toObject();
    // ffprobe returns nb_frames as a string in JSON (e.g., "24000"), not a number.
    // Must use toString() + QString::toInt() instead of QJsonValue::toInt().
    bool ok = false;
    int count = streamObj.value("nb_frames").toString().toInt(&ok);
    if (!ok || count <= 0) {
        qWarning() << "nb_frames not available in stream metadata for:" << fileName;
        return -1;
    }
    return count;
}

QList<ZFfprobe::StreamInfo> ZFfprobe::getMediaStreams(const QString& fileName)
{
    QList<StreamInfo> streams;
    
    if (fileName.isEmpty()) {
        return streams;
    }
    
    QString streamsJson = getMediaInfoJsonFormat(SHOW_STREAMS, fileName);
    if (streamsJson.isEmpty()) {
        return streams;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(streamsJson.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse streams JSON:" << error.errorString();
        return streams;
    }
    
    if (!doc.isObject()) {
        qWarning() << "Streams JSON is not an object";
        return streams;
    }
    
    QJsonObject rootObj = doc.object();
    if (!rootObj.contains("streams")) {
        qWarning() << "Streams JSON does not contain 'streams' field";
        return streams;
    }
    
    QJsonArray streamsArray = rootObj.value("streams").toArray();
    for (const QJsonValue& streamValue : streamsArray) {
        if (!streamValue.isObject()) {
            continue;
        }
        
        QJsonObject streamObj = streamValue.toObject();
        StreamInfo streamInfo;
        
        streamInfo.index = streamObj.value("index").toInt(-1);
        streamInfo.codecType = streamObj.value("codec_type").toString();
        streamInfo.codecName = streamObj.value("codec_name").toString();
        
        // Get tags for language and title if available
        QJsonObject tags = streamObj.value("tags").toObject();
        streamInfo.language = tags.value("language").toString();
        streamInfo.title = tags.value("title").toString();
        qDebug() << streamInfo.index << streamInfo.codecType << streamInfo.codecName;
        streams.append(streamInfo);
    }
    
    return streams;
}

QStringList ZFfprobe::getCodecsOrMuxersNames(const QString &key)
{
    QString cacheString;

    if (key == DECODER_FMT) {
        cacheString = getDecoders();
    } else if (key == ENCODER_FMT) {
        cacheString = getEncoders();
    } else if (key == DEMUXER_FMT) {
        cacheString = getDemuxers();
    } else if (key == MUXER_FMT) {
        cacheString = getMuxers();
    }

    QStringList cache = cacheString.split("\n", QT_SKIP_EMPTY_PARTS);

    if (cache.first().contains(":")) {
        cache.pop_front();
    }
    auto it = std::remove_if(cache.begin(), cache.end(),
                             [](const QString& protocol) {
                                 return protocol.contains("=") ||
                                        protocol.contains("--");
                             });
    cache.erase(it, cache.end());

    for (int i = 0; i < cache.size(); i++) {
        QStringList tmpList = cache.at(i).split(" ", QT_SKIP_EMPTY_PARTS);
        if (tmpList.size() > 1) {
            cache[i] = tmpList.at(1).trimmed();
        }
    }

    return cache;
}

QStringList ZFfprobe::getFiltersNames()
{
    QString filterString = getFilters();
    QStringList filters = filterString.split("\n", QT_SKIP_EMPTY_PARTS);

    auto it = std::remove_if(filters.begin(), filters.end(),
                             [](const QString& protocol) {
                                 return protocol.contains(":") ||
                                        protocol.contains("=") ||
                                        protocol.contains("--");
                             });
    filters.erase(it, filters.end());

    for (int i = 0; i < filters.size(); i++) {
        QStringList tmpList = filters.at(i).split(" ", QT_SKIP_EMPTY_PARTS);
        if (tmpList.size() > 1) {
            filters[i] = tmpList.at(1).trimmed();
        }
    }

    return filters;
}

QStringList ZFfprobe::getBsfsNames()
{
    QString bsfsString = getBsfs().replace(" ", "");
    QStringList bsfs = bsfsString.split("\n", QT_SKIP_EMPTY_PARTS);
    if (bsfs.first().contains(":")) {
        bsfs.pop_front();
    }

    return bsfs;
}

QStringList ZFfprobe::getProtocolNames()
{
    QString protocolString = getProtocols().replace(" ", "");
    QStringList protocols = protocolString.split("\n", QT_SKIP_EMPTY_PARTS);

    auto it = std::remove_if(protocols.begin(), protocols.end(),
                             [](const QString& protocol) {
                                 return protocol.contains(":");
                             });

    protocols.erase(it, protocols.end());

    return protocols;
}

QMap<QString, QList<QVariant>> ZFfprobe::getVideoSizeMap(const QString &key)
{
    static const QMap<QString, QList<QVariant>> videoSizes = {
        {"ntsc", {"720x480", 720, 480}},
        {"pal", {"720x576", 720, 576}},
        {"qntsc", {"352x240", 352, 240}},
        {"qpal", {"352x288", 352, 288}},
        {"sntsc", {"640x480", 640, 480}},
        {"spal", {"768x576", 768, 576}},
        {"film", {"352x240", 352, 240}},
        {"ntsc-film", {"352x240", 352, 240}},
        {"sqcif", {"128x96", 128, 96}},
        {"qcif", {"176x144", 176, 144}},
        {"cif", {"352x288", 352, 288}},
        {"4cif", {"704x576", 704, 576}},
        {"16cif", {"1408x1152", 1408, 1152}},
        {"qqvga", {"160x120", 160, 120}},
        {"qvga", {"320x240", 320, 240}},
        {"vga", {"640x480", 640, 480}},
        {"svga", {"800x600", 800, 600}},
        {"xga", {"1024x768", 1024, 768}},
        {"uxga", {"1600x1200", 1600, 1200}},
        {"qxga", {"2048x1536", 2048, 1536}},
        {"sxga", {"1280x1024", 1280, 1024}},
        {"qsxga", {"2560x2048", 2560, 2048}},
        {"hsxga", {"5120x4096", 5120, 4096}},
        {"wvga", {"852x480", 852, 480}},
        {"wxga", {"1366x768", 1366, 768}},
        {"wsxga", {"1600x1024", 1600, 1024}},
        {"wuxga", {"1920x1200", 1920, 1200}},
        {"woxga", {"2560x1600", 2560, 1600}},
        {"wqsxga", {"3200x2048", 3200, 2048}},
        {"wquxga", {"3840x2400", 3840, 2400}},
        {"whsxga", {"6400x4096", 6400, 4096}},
        {"whuxga", {"7680x4800", 7680, 4800}},
        {"cga", {"320x200", 320, 200}},
        {"ega", {"640x350", 640, 350}},
        {"hd480", {"852x480", 852, 480}},
        {"hd720", {"1280x720", 1280, 720}},
        {"hd1080", {"1920x1080", 1920, 1080}},
        {"2k", {"2048x1080", 2048, 1080}},
        {"2kflat", {"1998x1080", 1998, 1080}},
        {"2kscope", {"2048x858", 2048, 858}},
        {"4k", {"4096x2160", 4096, 2160}},
        {"4kflat", {"3996x2160", 3996, 2160}},
        {"4kscope", {"4096x1716", 4096, 1716}},
        {"nhd", {"640x360", 640, 360}},
        {"hqvga", {"240x160", 240, 160}},
        {"wqvga", {"400x240", 400, 240}},
        {"fwqvga", {"432x240", 432, 240}},
        {"hvga", {"480x320", 480, 320}},
        {"qhd", {"960x540", 960, 540}},
        {"2kdci", {"2048x1080", 2048, 1080}},
        {"4kdci", {"4096x2160", 4096, 2160}},
        {"uhd2160", {"3840x2160", 3840, 2160}},
        {"uhd4320", {"7680x4320", 7680, 4320}}
    };

    QMap<QString, QList<QVariant>> result;

    if (key.isEmpty()) {
        result = videoSizes;
    } else {
        auto it = videoSizes.find(key.toLower());
        if (it != videoSizes.end()) {
            result[it.key()] = it.value();
        }
    }

    return result;
}

QMap<QString, QList<QVariant>> ZFfprobe::getVideoRateMap(const QString &key)
{
    static const QMap<QString, QList<QVariant>> videoRates = {
        {"ntsc", {"30000/1001", 30000, 1001}},
        {"pal", {"25/1", 25, 1}},
        {"qntsc", {"30000/1001", 30000, 1001}},
        {"qpal", {"25/1", 25, 1}},
        {"sntsc", {"30000/1001", 30000, 1001}},
        {"spal", {"25/1", 25, 1}},
        {"film", {"24/1", 24, 1}},
        {"ntsc-film", {"24000/1001", 24000, 1001}}
    };

    QMap<QString, QList<QVariant>> result;

    if (key.isEmpty()) {
        result = videoRates;
    } else {
        auto it = videoRates.find(key.toLower());
        if (it != videoRates.end()) {
            result[it.key()] = it.value();
        }
    }

    return result;
}

QMap<QString, QList<QVariant>> ZFfprobe::getColor(const QString &key)
{
    static const QMap<QString, QPair<QString, QRgb>> colors = {
        {"AliceBlue", {"0xF0F8FF", 0xF0F8FF}},
        {"AntiqueWhite", {"0xFAEBD7", 0xFAEBD7}},
        {"Aqua", {"0x00FFFF", 0x00FFFF}},
        {"Aquamarine", {"0x7FFFD4", 0x7FFFD4}},
        {"Azure", {"0xF0FFFF", 0xF0FFFF}},
        {"Beige", {"0xF5F5DC", 0xF5F5DC}},
        {"Bisque", {"0xFFE4C4", 0xFFE4C4}},
        {"Black", {"0x000000", 0x000000}},
        {"BlanchedAlmond", {"0xFFEBCD", 0xFFEBCD}},
        {"Blue", {"0x0000FF", 0x0000FF}},
        {"BlueViolet", {"0x8A2BE2", 0x8A2BE2}},
        {"Brown", {"0xA52A2A", 0xA52A2A}},
        {"BurlyWood", {"0xDEB887", 0xDEB887}},
        {"CadetBlue", {"0x5F9EA0", 0x5F9EA0}},
        {"Chartreuse", {"0x7FFF00", 0x7FFF00}},
        {"Chocolate", {"0xD2691E", 0xD2691E}},
        {"Coral", {"0xFF7F50", 0xFF7F50}},
        {"CornflowerBlue", {"0x6495ED", 0x6495ED}},
        {"Cornsilk", {"0xFFF8DC", 0xFFF8DC}},
        {"Crimson", {"0xDC143C", 0xDC143C}},
        {"Cyan", {"0x00FFFF", 0x00FFFF}},
        {"DarkBlue", {"0x00008B", 0x00008B}},
        {"DarkCyan", {"0x008B8B", 0x008B8B}},
        {"DarkGoldenRod", {"0xB8860B", 0xB8860B}},
        {"DarkGray", {"0xA9A9A9", 0xA9A9A9}},
        {"DarkGreen", {"0x006400", 0x006400}},
        {"DarkKhaki", {"0xBDB76B", 0xBDB76B}},
        {"DarkMagenta", {"0x8B008B", 0x8B008B}},
        {"DarkOliveGreen", {"0x556B2F", 0x556B2F}},
        {"Darkorange", {"0xFF8C00", 0xFF8C00}},
        {"DarkOrchid", {"0x9932CC", 0x9932CC}},
        {"DarkRed", {"0x8B0000", 0x8B0000}},
        {"DarkSalmon", {"0xE9967A", 0xE9967A}},
        {"DarkSeaGreen", {"0x8FBC8F", 0x8FBC8F}},
        {"DarkSlateBlue", {"0x483D8B", 0x483D8B}},
        {"DarkSlateGray", {"0x2F4F4F", 0x2F4F4F}},
        {"DarkTurquoise", {"0x00CED1", 0x00CED1}},
        {"DarkViolet", {"0x9400D3", 0x9400D3}},
        {"DeepPink", {"0xFF1493", 0xFF1493}},
        {"DeepSkyBlue", {"0x00BFFF", 0x00BFFF}},
        {"DimGray", {"0x696969", 0x696969}},
        {"DodgerBlue", {"0x1E90FF", 0x1E90FF}},
        {"FireBrick", {"0xB22222", 0xB22222}},
        {"FloralWhite", {"0xFFFAF0", 0xFFFAF0}},
        {"ForestGreen", {"0x228B22", 0x228B22}},
        {"Fuchsia", {"0xFF00FF", 0xFF00FF}},
        {"Gainsboro", {"0xDCDCDC", 0xDCDCDC}},
        {"GhostWhite", {"0xF8F8FF", 0xF8F8FF}},
        {"Gold", {"0xFFD700", 0xFFD700}},
        {"GoldenRod", {"0xDAA520", 0xDAA520}},
        {"Gray", {"0x808080", 0x808080}},
        {"Green", {"0x008000", 0x008000}},
        {"GreenYellow", {"0xADFF2F", 0xADFF2F}},
        {"HoneyDew", {"0xF0FFF0", 0xF0FFF0}},
        {"HotPink", {"0xFF69B4", 0xFF69B4}},
        {"IndianRed", {"0xCD5C5C", 0xCD5C5C}},
        {"Indigo", {"0x4B0082", 0x4B0082}},
        {"Ivory", {"0xFFFFF0", 0xFFFFF0}},
        {"Khaki", {"0xF0E68C", 0xF0E68C}},
        {"Lavender", {"0xE6E6FA", 0xE6E6FA}},
        {"LavenderBlush", {"0xFFF0F5", 0xFFF0F5}},
        {"LawnGreen", {"0x7CFC00", 0x7CFC00}},
        {"LemonChiffon", {"0xFFFACD", 0xFFFACD}},
        {"LightBlue", {"0xADD8E6", 0xADD8E6}},
        {"LightCoral", {"0xF08080", 0xF08080}},
        {"LightCyan", {"0xE0FFFF", 0xE0FFFF}},
        {"LightGoldenRodYellow", {"0xFAFAD2", 0xFAFAD2}},
        {"LightGreen", {"0x90EE90", 0x90EE90}},
        {"LightGrey", {"0xD3D3D3", 0xD3D3D3}},
        {"LightPink", {"0xFFB6C1", 0xFFB6C1}},
        {"LightSalmon", {"0xFFA07A", 0xFFA07A}},
        {"LightSeaGreen", {"0x20B2AA", 0x20B2AA}},
        {"LightSkyBlue", {"0x87CEFA", 0x87CEFA}},
        {"LightSlateGray", {"0x778899", 0x778899}},
        {"LightSteelBlue", {"0xB0C4DE", 0xB0C4DE}},
        {"LightYellow", {"0xFFFFE0", 0xFFFFE0}},
        {"Lime", {"0x00FF00", 0x00FF00}},
        {"LimeGreen", {"0x32CD32", 0x32CD32}},
        {"Linen", {"0xFAF0E6", 0xFAF0E6}},
        {"Magenta", {"0xFF00FF", 0xFF00FF}},
        {"Maroon", {"0x800000", 0x800000}},
        {"MediumAquaMarine", {"0x66CDAA", 0x66CDAA}},
        {"MediumBlue", {"0x0000CD", 0x0000CD}},
        {"MediumOrchid", {"0xBA55D3", 0xBA55D3}},
        {"MediumPurple", {"0x9370D8", 0x9370D8}},
        {"MediumSeaGreen", {"0x3CB371", 0x3CB371}},
        {"MediumSlateBlue", {"0x7B68EE", 0x7B68EE}},
        {"MediumSpringGreen", {"0x00FA9A", 0x00FA9A}},
        {"MediumTurquoise", {"0x48D1CC", 0x48D1CC}},
        {"MediumVioletRed", {"0xC71585", 0xC71585}},
        {"MidnightBlue", {"0x191970", 0x191970}},
        {"MintCream", {"0xF5FFFA", 0xF5FFFA}},
        {"MistyRose", {"0xFFE4E1", 0xFFE4E1}},
        {"Moccasin", {"0xFFE4B5", 0xFFE4B5}},
        {"NavajoWhite", {"0xFFDEAD", 0xFFDEAD}},
        {"Navy", {"0x000080", 0x000080}},
        {"OldLace", {"0xFDF5E6", 0xFDF5E6}},
        {"Olive", {"0x808000", 0x808000}},
        {"OliveDrab", {"0x6B8E23", 0x6B8E23}},
        {"Orange", {"0xFFA500", 0xFFA500}},
        {"OrangeRed", {"0xFF4500", 0xFF4500}},
        {"Orchid", {"0xDA70D6", 0xDA70D6}},
        {"PaleGoldenRod", {"0xEEE8AA", 0xEEE8AA}},
        {"PaleGreen", {"0x98FB98", 0x98FB98}},
        {"PaleTurquoise", {"0xAFEEEE", 0xAFEEEE}},
        {"PaleVioletRed", {"0xD87093", 0xD87093}},
        {"PapayaWhip", {"0xFFEFD5", 0xFFEFD5}},
        {"PeachPuff", {"0xFFDAB9", 0xFFDAB9}},
        {"Peru", {"0xCD853F", 0xCD853F}},
        {"Pink", {"0xFFC0CB", 0xFFC0CB}},
        {"Plum", {"0xDDA0DD", 0xDDA0DD}},
        {"PowderBlue", {"0xB0E0E6", 0xB0E0E6}},
        {"Purple", {"0x800080", 0x800080}},
        {"Red", {"0xFF0000", 0xFF0000}},
        {"RosyBrown", {"0xBC8F8F", 0xBC8F8F}},
        {"RoyalBlue", {"0x4169E1", 0x4169E1}},
        {"SaddleBrown", {"0x8B4513", 0x8B4513}},
        {"Salmon", {"0xFA8072", 0xFA8072}},
        {"SandyBrown", {"0xF4A460", 0xF4A460}},
        {"SeaGreen", {"0x2E8B57", 0x2E8B57}},
        {"SeaShell", {"0xFFF5EE", 0xFFF5EE}},
        {"Sienna", {"0xA0522D", 0xA0522D}},
        {"Silver", {"0xC0C0C0", 0xC0C0C0}},
        {"SkyBlue", {"0x87CEEB", 0x87CEEB}},
        {"SlateBlue", {"0x6A5ACD", 0x6A5ACD}},
        {"SlateGray", {"0x708090", 0x708090}},
        {"Snow", {"0xFFFAFA", 0xFFFAFA}},
        {"SpringGreen", {"0x00FF7F", 0x00FF7F}},
        {"SteelBlue", {"0x4682B4", 0x4682B4}},
        {"Tan", {"0xD2B48C", 0xD2B48C}},
        {"Teal", {"0x008080", 0x008080}},
        {"Thistle", {"0xD8BFD8", 0xD8BFD8}},
        {"Tomato", {"0xFF6347", 0xFF6347}},
        {"Turquoise", {"0x40E0D0", 0x40E0D0}},
        {"Violet", {"0xEE82EE", 0xEE82EE}},
        {"Wheat", {"0xF5DEB3", 0xF5DEB3}},
        {"White", {"0xFFFFFF", 0xFFFFFF}},
        {"WhiteSmoke", {"0xF5F5F5", 0xF5F5F5}},
        {"Yellow", {"0xFFFF00", 0xFFFF00}},
        {"YellowGreen", {"0x9ACD32", 0x9ACD32}}
    };

    QMap<QString, QList<QVariant>> result;

    if (key.isEmpty()) {
        for (auto it = colors.begin(); it != colors.end(); ++it) {
            QList<QVariant> colorInfo;
            colorInfo << it.value().first << it.value().second;
            result[it.key()] = colorInfo;
        }
    } else {
        auto it = colors.find(key);
        if (it != colors.end()) {
            QList<QVariant> colorInfo;
            colorInfo << it.value().first << it.value().second;
            result[it.key()] = colorInfo;
        }
    }

    return result;
}

FormattedTableData ZFfprobe::formatBasicInfoToTable(const QString &data, const QString &formatKey)
{
    FormattedTableData result;

    QStringList rawStringLines = data.split("\n", QT_SKIP_EMPTY_PARTS);

    if (rawStringLines.size() < 1)
        return result;

    if (rawStringLines.at(0).contains(":"))
        rawStringLines.removeFirst();

    // -L -sources -sinks
    if (QStringList{"L", "sources", "sinks"}.contains(formatKey, Qt::CaseInsensitive)){
        result.headers << "Info";
        for (auto it : rawStringLines) {
            result.rows << QStringList{it};
        }
        return result;
    }

    // -version
    if (QStringList{"version"}.contains(formatKey, Qt::CaseInsensitive)) {
        result.headers << "Config" << "Value";

        result.rows.append(QStringList{"Version", rawStringLines.at(0)});
        result.rows.append(QStringList{"Build", rawStringLines.at(1)});

        for (int i = 2; i < rawStringLines.size(); ++i) {
            QStringList tmp = rawStringLines.at(i).split(" ", QT_SKIP_EMPTY_PARTS);
            if (tmp.size() > 0 && tmp.at(0).contains("configuration"))
                tmp.takeFirst();

            QStringList tb_of_line;
            if (rawStringLines.at(i).contains("=")){
                for (auto it : tmp) {
                    QStringList keyParts = it.split("=", QT_SKIP_EMPTY_PARTS);
                    tb_of_line.clear();
                    if (keyParts.size() == 2) {
                        tb_of_line << keyParts[0].replace("--", "").trimmed() << keyParts.at(1).trimmed();
                    } else {
                        tb_of_line << "" << keyParts[0].replace("--", "").trimmed();
                    }
                    result.rows.append(tb_of_line);
                }
            } else {
                tb_of_line.append(tmp.at(0).trimmed());
                tmp.removeFirst();
                tb_of_line.append(tmp.join(" "));
                result.rows.append(tb_of_line);
            }
        }
        return result;
    }

    // -formats -muxers -demuxers -devices -codecs -decoders -encoders -filters -pix_fmts
    if (QStringList{"formats", "muxers", "demuxers", "devices", "codecs", "decoders", "filters", "encoders", "pixfmts"}.contains(formatKey, Qt::CaseInsensitive)) {
        QStringList parts;
        parts << QString("") << QString("");
        if (formatKey.contains("filters")) {
            for (auto it : rawStringLines) {
                if (it.contains("=")) {
                    parts[0].append(it + "\n");
                } else {
                    parts[1].append(it + "\n");
                }
            }
        } else {
            parts = data.split("--", QT_SKIP_EMPTY_PARTS);
        }

        if (parts.size() >= 2) {

            // header
            QString head_code = "";
            auto tmp_headers = parts.at(0).split("\n", QT_SKIP_EMPTY_PARTS);
            for (auto it : tmp_headers) {
                if (it.contains(":"))
                    continue;

                if (it.contains("=")) {
                    QStringList tmp = it.split("=", QT_SKIP_EMPTY_PARTS);
                    if (tmp.size() >= 2) {
                        result.headers.append("[" +tmp[0].replace(".", "").trimmed() + "]" + tmp.at(1).trimmed());
                        head_code.append(tmp[0].replace(".", "").trimmed());
                    }
                }
            }
            result.headers.append("name");
            if (formatKey.contains("pixfmts")) {
                result.headers.append("NB_COMPONENTS");
                result.headers.append("BITS_PER_PIXEL");
            } else if (formatKey.contains("filters")){
                result.headers.append("direct");
            } else {
                result.headers.append("detail");
            }

            qDebug() << "header size:" << result.headers.size() << "head_code: " << head_code;

            // content
            auto tmp_content = parts.at(1).split("\n", QT_SKIP_EMPTY_PARTS);
            for (auto it : tmp_content) {
                QStringList tb_of_line;

                QStringList tmp = it.split(" ", QT_SKIP_EMPTY_PARTS);
                if (tmp.size() < 3){
                    continue;
                }
                for (int ch = 0; ch < head_code.size(); ++ch) {
                    tb_of_line.append("");
                }
                for (auto ch : tmp.at(0).trimmed()) {
                    int index = head_code.lastIndexOf(ch);

                    if (index >= 0) {
                        tb_of_line[index] = "√";
                    }
                }

                tb_of_line.append(tmp.at(1).trimmed());

                tmp.removeFirst();
                tmp.removeFirst();

                if (formatKey.contains("pixfmts")) {
                    tb_of_line.append(tmp.at(0).trimmed());
                    tb_of_line.append(tmp.at(1).trimmed());
                }else {
                    tb_of_line.append(tmp.join(" "));
                }

                result.rows.append(tb_of_line);
            }
        }

        return result;
    }

    // -colors -sample_fmts -layouts (inner: videorate, videosize)
    if (QStringList{"colors", "samplefmts", "layouts", "videorate", "videosize"}.contains(formatKey, Qt::CaseInsensitive)) {
        result.headers << "Name" << "Value";

        for (int i = 1; i < rawStringLines.size(); ++i) {
            if (rawStringLines.at(i).contains(":", Qt::CaseInsensitive) ||
                rawStringLines.at(i).contains("DECOMPOSITION", Qt::CaseInsensitive) ||
                rawStringLines.at(i).contains("DESCRIPTION", Qt::CaseInsensitive)) {
                continue;
            }
            QStringList tmp = rawStringLines.at(i).split(" ", QT_SKIP_EMPTY_PARTS);
            if (tmp.size() >= 2) {
                QStringList tb_of_line;
                tb_of_line << tmp.at(0).trimmed();
                tmp.removeFirst();
                tb_of_line.append(tmp.join(" "));
                result.rows.append(tb_of_line);
            }
        }

        return result;
    }

    // -protocols
    if (QStringList{"protocols"}.contains(formatKey, Qt::CaseInsensitive)) {
        result.headers << "Input" << "Output";

        QStringList input, output;
        bool startOutput = false;
        for (int i = 2; i < rawStringLines.size(); i++) {
            if (rawStringLines.at(i).contains("output", Qt::CaseInsensitive)){
                startOutput = true;
                continue;
            }
            if (startOutput){
                output.append(rawStringLines.at(i).trimmed());
            } else {
                input.append(rawStringLines.at(i).trimmed());
            }
        }

        int maxRow = std::max(input.size(), output.size());

        for (int i = 0; i < maxRow; i++) {
            QStringList tmpRow;
            if (input.size() <= i) {
                tmpRow.append("");
            } else {
                tmpRow.append(input.at(i));
            }

            if (output.size() <= i) {
                tmpRow.append("");
            } else {
                tmpRow.append(output.at(i));
            }

            result.rows.append(tmpRow);
        }

        return result;
    }

    // -bsfs -buildconf
    if (QStringList{"bsfs", "buildconf"}.contains(formatKey, Qt::CaseInsensitive)) {
        result.headers.append("name");

        for (int i = 0; i < rawStringLines.size(); i++) {
            result.rows.append(QStringList{rawStringLines.at(i).trimmed()});
        }

        return result;
    }

    return result;
}

QString ZFfprobe::getFFprobeCommandOutput(const QString &command, const QStringList &otherParms)
{
    QProcess process;
    process.start(FFPROBE, QStringList() << ffmpegCommandList <<
                               command << otherParms);

    qDebug() << "cmd: " << process.arguments().join(" ").prepend(" ").prepend(FFPROBE);
    process.waitForFinished(-1);
    return process.readAll();
}
