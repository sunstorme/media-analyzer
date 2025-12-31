// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "common.h"
#include <QProcess>
#include <QStandardPaths>

QList<QStringList> Common::logLevels = {
    {"quiet", "-8", "Show nothing at all; be silent."},
    {"panic", "0", "Only show fatal errors which could lead the process to crash, such as an assertion failure. This is not currently used for anything."},
    {"fatal", "8", "Only show fatal errors. These are errors after which the process absolutely cannot continue."},
    {"error", "16", "Show all errors, including ones which can be recovered from."},
    {"warning", "24", "Show all warnings and errors. Any message related to possibly incorrect or unexpected events will be shown."},
    {"info", "32", "Show informative messages during processing. This is in addition to warnings and errors. This is the default value."},
    {"verbose", "40", "Same as info, except more verbose."},
    {"debug", "48", "Show everything, including debugging information."},
    {"trace", "56", "shows extremely detailed, low-level tracing information about the internal execution flow of the code, allowing for deep technical debugging and analysis."}
};

const QStringList CONFIG_GROUPS = {
    LOG_SETTINGS_GROUP,
    GENERAL_SETTINGS_GROUP,
    MAINWINDOW_SETTINGS_GROUP,
    WINDOWHEAD_SETTINGS_GROUP,
    RECENTFILES_SETTINGS_GROUP
};

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

QSet<QString> Common::supportedVideoMimeTypes()
{
    return getSupportedVideoMimeTypes();
}

QSet<QString> Common::supportedVideoExtensions()
{
    return getSupportedVideoExtensions();
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
        if (!filePath.isEmpty() && isSupportedVideoFile(filePath)) {
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
        if (!filePath.isEmpty() && isSupportedVideoFile(filePath)) {
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
    QString command = QString("dpkg -s %1 | grep Version | awk '{print $2}'").arg(packageName);

    process.start("bash", QStringList() << "-c" << command);
    process.waitForFinished(3000); // Wait up to 3 seconds

    QString output = process.readAllStandardOutput().trimmed();
    QString error = process.readAllStandardError().trimmed();

    if (!error.isEmpty()) {
        qWarning() << "Error getting package version for" << packageName << ":" << error;
        return QString();
    }

    return output;
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
