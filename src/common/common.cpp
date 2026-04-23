// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "common.h"
#include <QProcess>
#include <QStandardPaths>

QList<QStringList> Common::logLevels = QList<QStringList>()
    << (QStringList() << "quiet" << "-8" << "Show nothing at all; be silent.")
    << (QStringList() << "panic" << "0" << "Only show fatal errors which could lead the process to crash, such as an assertion failure. This is not currently used for anything.")
    << (QStringList() << "fatal" << "8" << "Only show fatal errors. These are errors after which the process absolutely cannot continue.")
    << (QStringList() << "error" << "16" << "Show all errors, including ones which can be recovered from.")
    << (QStringList() << "warning" << "24" << "Show all warnings and errors. Any message related to possibly incorrect or unexpected events will be shown.")
    << (QStringList() << "info" << "32" << "Show informative messages during processing. This is in addition to warnings and errors. This is the default value.")
    << (QStringList() << "verbose" << "40" << "Same as info, except more verbose.")
    << (QStringList() << "debug" << "48" << "Show everything, including debugging information.")
    << (QStringList() << "trace" << "56" << "shows extremely detailed, low-level tracing information about the internal execution flow of the code, allowing for deep technical debugging and analysis.");

const QStringList CONFIG_GROUPS = QStringList()
    << LOG_SETTINGS_GROUP
    << GENERAL_SETTINGS_GROUP;

QString translatedConfigGroupName(const QString &groupKey)
{
    if (groupKey == LOG_SETTINGS_GROUP)
        return QObject::tr("Log");
    if (groupKey == GENERAL_SETTINGS_GROUP)
        return QObject::tr("General");
    return groupKey;
}

QMap<QString, QString> translatedConfigGroups()
{
    QMap<QString, QString> result;
    for (const auto &key : CONFIG_GROUPS) {
        result.insert(key, translatedConfigGroupName(key));
    }
    return result;
}

QMutex Common::m_mutex;
Common* Common::m_instance = nullptr;

Common::Common()
    : m_initialized(false)
{
    // Private constructor
}

Common::~Common()
{
    // Cleanup resources
    qDebug() << "Common singleton destroyed";
}

Common* Common::instance()
{
    QMutexLocker locker(&m_mutex);
    if (!m_instance) {
        m_instance = new Common();
    }
    return m_instance;
}

void Common::destroy()
{
    QMutexLocker locker(&m_mutex);
    delete m_instance;
    m_instance = nullptr;
}

void Common::setConfigValue(const QString& key, const QVariant& value)
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    // Set encoding to UTF-8 to support Chinese paths (Qt5 only)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    settings.setIniCodec("UTF-8");
#endif
    settings.setValue(key, value);
    // Sync immediately to ensure configuration is written
    settings.sync();
}

QVariant Common::getConfigValue(const QString& key, const QVariant& defaultValue) const
{
    QSettings settings(ORGANIZATION_NAME, APPLICATION_NAME);
    // Set encoding to UTF-8 to support Chinese paths (Qt5 only)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    settings.setIniCodec("UTF-8");
#endif
    return settings.value(key, defaultValue);
}

void Common::initialize()
{
    if (m_initialized) {
        return;
    }
    m_initialized = true;
}

bool Common::isInitialized() const
{
    return m_initialized;
}

bool Common::isSupportedVideoFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.isFile() || !fileInfo.exists()) {
        return false;
    }

    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(filePath, QMimeDatabase::MatchExtension);

    const QSet<QString> &supportedMimeTypes = getSupportedVideoMimeTypes();
    if (supportedMimeTypes.contains(mimeType.name())) {
        return true;
    }

    const QSet<QString> &supportedExtensions = getSupportedVideoExtensions();
    QString extension = fileInfo.suffix().toLower();
    return supportedExtensions.contains(extension);
}

bool Common::isVideoFile(const QString &filePath)
{
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(filePath);

    return mimeType.name().startsWith("video/");
}

bool Common::isAudioFile(const QString &filePath)
{
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(filePath);

    return mimeType.name().startsWith("audio/");
}

bool Common::isMediaFile(const QString &filePath)
{
    return isVideoFile(filePath) || isAudioFile(filePath);
}

bool Common::isImageFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.isFile() || !fileInfo.exists()) {
        return false;
    }

    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(filePath, QMimeDatabase::MatchExtension);

    const QSet<QString> &supportedMimeTypes = getSupportedImageMimeTypes();
    if (supportedMimeTypes.contains(mimeType.name())) {
        return true;
    }

    const QSet<QString> &supportedExtensions = getSupportedImageExtensions();
    QString extension = fileInfo.suffix().toLower();
    return supportedExtensions.contains(extension);
}

bool Common::isSubtitleFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.isFile() || !fileInfo.exists()) {
        return false;
    }

    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(filePath, QMimeDatabase::MatchExtension);

    const QSet<QString> &supportedMimeTypes = getSupportedSubtitleMimeTypes();
    if (supportedMimeTypes.contains(mimeType.name())) {
        return true;
    }

    const QSet<QString> &supportedExtensions = getSupportedSubtitleExtensions();
    QString extension = fileInfo.suffix().toLower();
    return supportedExtensions.contains(extension);
}

bool Common::isSupportedMediaFile(const QString &filePath)
{
    return isSupportedVideoFile(filePath) || isAudioFile(filePath) || 
           isImageFile(filePath) || isSubtitleFile(filePath);
}

QSet<QString> Common::supportedVideoMimeTypes()
{
    return getSupportedVideoMimeTypes();
}

QSet<QString> Common::supportedVideoExtensions()
{
    return getSupportedVideoExtensions();
}

QSet<QString> Common::supportedAudioMimeTypes()
{
    return getSupportedAudioMimeTypes();
}

QSet<QString> Common::supportedAudioExtensions()
{
    return getSupportedAudioExtensions();
}

QSet<QString> Common::supportedImageMimeTypes()
{
    return getSupportedImageMimeTypes();
}

QSet<QString> Common::supportedImageExtensions()
{
    return getSupportedImageExtensions();
}

QSet<QString> Common::supportedSubtitleMimeTypes()
{
    return getSupportedSubtitleMimeTypes();
}

QSet<QString> Common::supportedSubtitleExtensions()
{
    return getSupportedSubtitleExtensions();
}

QStringList Common::extractSupportedMediaFiles(const QMimeData *mimeData)
{
    QStringList supportedFiles;

    if (!mimeData || !mimeData->hasUrls()) {
        return supportedFiles;
    }

    QList<QUrl> urlList = mimeData->urls();
    for (const QUrl &url : urlList) {
        QString filePath = url.toLocalFile();
        if (!filePath.isEmpty() && isSupportedMediaFile(filePath)) {
            supportedFiles.append(filePath);
        }
    }

    return supportedFiles;
}

bool Common::containsSupportedMediaFiles(const QMimeData *mimeData)
{
    if (!mimeData || !mimeData->hasUrls()) {
        return false;
    }

    QList<QUrl> urlList = mimeData->urls();
    for (const QUrl &url : urlList) {
        QString filePath = url.toLocalFile();
        if (!filePath.isEmpty() && isSupportedMediaFile(filePath)) {
            return true;
        }
    }

    return false;
}

const QSet<QString> &Common::getSupportedVideoMimeTypes()
{
    static QSet<QString> supportedMimeTypes = {
        "video/mp4",
        "video/x-msvideo",    // AVI
        "video/x-matroska",   // MKV
        "video/quicktime",    // MOV
        "video/x-ms-wmv",     // WMV
        "video/mpeg",         // MPG, MPEG
        "video/x-flv",        // FLV
        "video/webm",         // WEBM
        "video/3gpp",         // 3GP
        "video/mp2t",         // TS, M2TS
        "video/x-m4v",        // M4V
        "video/x-ms-asf",     // ASF
        "video/x-mng",        // MNG
        "video/x-sgi-movie"   // MOV
    };

    return supportedMimeTypes;
}

const QSet<QString> &Common::getSupportedVideoExtensions()
{
    static QSet<QString> supportedExtensions = {
        "mp4", "avi", "mkv", "mov", "wmv", "mpg", "mpeg",
        "flv", "webm", "m4v", "3gp", "ts", "mts", "m2ts",
        "asf", "mng", "qt", "divx", "xvid", "rm", "rmvb",
        "vob", "ogv", "mxf", "mjp", "mjpeg"
    };

    return supportedExtensions;
}

const QSet<QString> &Common::getSupportedAudioMimeTypes()
{
    static QSet<QString> supportedMimeTypes = {
        "audio/mpeg",        // MP3
        "audio/mp4",         // M4A
        "audio/x-m4a",       // M4A
        "audio/wav",         // WAV
        "audio/wave",        // WAV
        "audio/x-wav",       // WAV
        "audio/ogg",         // OGG
        "audio/flac",        // FLAC
        "audio/x-flac",      // FLAC
        "audio/aac",         // AAC
        "audio/aacp",        // AAC+
        "audio/x-aac",       // AAC
        "audio/x-ms-wma",    // WMA
        "audio/vnd.wave",    // WAV
        "audio/x-aiff",      // AIFF
        "audio/aiff",        // AIFF
        "audio/basic",       // AU
        "audio/x-au",        // AU
        "audio/midi",        // MIDI
        "audio/x-midi",      // MIDI
        "audio/opus",        // OPUS
        "audio/x-opus",      // OPUS
        "audio/x-speex",     // SPEEX
        "audio/x-vorbis",    // VORBIS
        "audio/x-vorbis+ogg" // VORBIS
    };

    return supportedMimeTypes;
}

const QSet<QString> &Common::getSupportedAudioExtensions()
{
    static QSet<QString> supportedExtensions = {
        "mp3", "m4a", "wav", "wave", "ogg", "oga", "flac",
        "aac", "wma", "aiff", "aif", "au", "snd", "mid", "midi",
        "rmi", "opus", "speex", "amr", "awb", "ape", "ac3",
        "dts", "dtshd", "tta", "wv", "ast", "au", "gsm"
    };

    return supportedExtensions;
}

const QSet<QString> &Common::getSupportedImageMimeTypes()
{
    static QSet<QString> supportedMimeTypes = {
        "image/jpeg",        // JPG, JPEG
        "image/png",         // PNG
        "image/gif",         // GIF
        "image/webp",        // WEBP
        "image/bmp",         // BMP
        "image/x-bmp",       // BMP
        "image/x-ms-bmp",    // BMP
        "image/tiff",        // TIFF, TIF
        "image/x-tiff",      // TIFF
        "image/svg+xml",     // SVG
        "image/vnd.microsoft.icon",  // ICO
        "image/x-icon",      // ICO
        "image/x-ico",       // ICO
        "image/vnd.adobe.photoshop",  // PSD
        "image/x-photoshop", // PSD
        "image/x-psd",       // PSD
        "image/x-portable-pixmap",  // PPM
        "image/x-portable-graymap", // PGM
        "image/x-portable-bitmap",  // PBM
        "image/x-pcx",       // PCX
        "image/x-tga",       // TGA
        "image/x-icns",      // ICNS
        "image/jp2",         // JPEG 2000
        "image/x-jp2",       // JPEG 2000
        "image/jpx",         // JPEG 2000
        "image/apng",        // APNG
        "image/avif",        // AVIF
        "image/heic",        // HEIC
        "image/heif",        // HEIF
        "image/x-heic",      // HEIC
        "image/x-heif"       // HEIF
    };

    return supportedMimeTypes;
}

const QSet<QString> &Common::getSupportedImageExtensions()
{
    static QSet<QString> supportedExtensions = {
        "jpg", "jpeg", "png", "gif", "webp", "bmp", "dib",
        "tiff", "tif", "svg", "svgz", "ico", "cur", "psd",
        "psb", "ppm", "pgm", "pbm", "pnm", "pcx", "tga",
        "icns", "jp2", "j2k", "jpf", "jpx", "apng", "avif",
        "heic", "heif", "jxl", "exr", "hdr", "pic", "pict",
        "pct", "pix", "pxr", "raw", "cr2", "crw", "nef",
        "orf", "pef", "raf", "sr2", "dng", "arw", "k25"
    };

    return supportedExtensions;
}

const QSet<QString> &Common::getSupportedSubtitleMimeTypes()
{
    static QSet<QString> supportedMimeTypes = {
        "text/plain",        // Generic text (for SRT, ASS, SSA, VTT, etc.)
        "text/x-ssa",        // SSA
        "text/x-ass",        // ASS
        "text/x-srt",        // SRT
        "text/x-vtt",        // VTT
        "application/x-subrip",  // SRT
        "application/x-subtitle" // Generic subtitle
    };

    return supportedMimeTypes;
}

const QSet<QString> &Common::getSupportedSubtitleExtensions()
{
    static QSet<QString> supportedExtensions = {
        "srt",      // SubRip
        "ass",      // Advanced SubStation Alpha
        "ssa",      // SubStation Alpha
        "vtt",      // WebVTT
        "sub",      // MicroDVD
        "idx",      // VobSub index
        "smi",      // SAMI
        "sam",      // SAMI
        "psb",      // PowerScript
        "rt",       // RealText
        "txt",      // Plain text (may contain subtitles)
        "usf",      // Universal Subtitle Format
        "jss",      // JACOsub
        "ssf",      // Stretched Sprite Format
        "sup",      // HDMV Presentation Graphic Stream
        "mks",      // MKA subtitle
        "lrc",      // LRC lyrics
        "sbv",      // YouTube subtitles
        "xml",      // TTML, etc.
        "ttml",     // TTML
        "dfxp",     // Distribution Format Exchange Profile
        "cap",      // CEA-608
        "sc",       // Scenarist
        "scc",      // Scenarist Closed Captions
        "stl",      // Spruce Subtitle
        "son",      // Son
        "s2k",      // Son
        "pjs",      // Phoenix Subtitle
        "mpl",      // MPL2
        "mpsub",    // MPlayer subtitle
        "aqtitle",  // AQTitle
        "gsub",     // GSUB
        "js",       // JSON Subtitle
        "json"      // JSON Subtitle
    };

    return supportedExtensions;
}

QAction *Common::findActionByObjectName(QMenu *menu, const QString &objectName)
{
    if (!menu) {
        return nullptr;
    }
    
    // Search through all actions in the menu
    QList<QAction *> actions = menu->actions();
    for (QAction *action : actions) {
        if (action && action->objectName() == objectName) {
            return action;
        }
        
        // Also check sub-menus recursively
        if (action && action->menu()) {
            QAction *foundAction = findActionByObjectName(action->menu(), objectName);
            if (foundAction) {
                return foundAction;
            }
        }
    }
    
    return nullptr;
}

QAction *Common::findActionByText(QMenu *menu, const QString &text)
{
    if (!menu) {
        return nullptr;
    }
    
    // Search through all actions in the menu
    QList<QAction *> actions = menu->actions();
    for (QAction *action : actions) {
        if (action && action->text() == text) {
            return action;
        }
        
        // Also check sub-menus recursively
        if (action && action->menu()) {
            QAction *foundAction = findActionByText(action->menu(), text);
            if (foundAction) {
                return foundAction;
            }
        }
    }
    
    return nullptr;
}

QList<QAction *> Common::getAllActions(QMenu *menu)
{
    if (!menu) {
        return QList<QAction *>();
    }
    
    QList<QAction *> allActions;
    
    // Get all actions from the menu
    QList<QAction *> actions = menu->actions();
    for (QAction *action : actions) {
        if (action) {
            allActions.append(action);
            
            // Recursively get actions from sub-menus
            if (action->menu()) {
                QList<QAction *> subActions = getAllActions(action->menu());
                allActions.append(subActions);
            }
        }
    }
    
    return allActions;
}

bool Common::ensureDirectory(const QString& dirPath)
{
    QDir dir(dirPath);
    
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create directory:" << dirPath;
            return false;
        }
        qDebug() << "Created directory:" << dirPath;
    }
    
    return true;
}

void Common::cleanupDirectory(const QString& dirPath)
{
    QDir dir(dirPath);
    
    if (dir.exists()) {
        bool success = dir.removeRecursively();
        if (success) {
            qDebug() << "Successfully cleaned up directory:" << dirPath;
        } else {
            qWarning() << "Failed to clean up directory:" << dirPath;
        }
    }
}


QString Common::getPackageVersion(const QString& packageName)
{
    QProcess process;
    // Use QProcess arguments directly to avoid shell injection via packageName
    process.start("dpkg", QStringList() << "-s" << packageName);
    process.waitForFinished(3000); // Wait up to 3 seconds

    QString output = process.readAllStandardOutput();
    QString error = process.readAllStandardError().trimmed();

    if (!error.isEmpty()) {
        qWarning() << "Error getting package version for" << packageName << ":" << error;
        return QString();
    }

    // Parse the "Version:" line from dpkg output (replaces grep+awk pipe)
    for (const QString& line : output.split('\n')) {
        if (line.startsWith("Version:")) {
            return line.mid(8).trimmed();
        }
    }

    return QString();
}

QString Common::getCurrentPackageVersion()
{
    // Try to determine which package is currently running
    QString appPath = QCoreApplication::applicationFilePath();

    // Check if we're running media-analyzer or media-analyzer6
    if (appPath.contains("media-analyzer6")) {
        return getPackageVersion("media-analyzer6");
    } else {
        return getPackageVersion("media-analyzer");
    }
}
