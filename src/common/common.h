// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QMutex>
#include <QSettings>
#include <QSet>
#include <QMap>
#include <QDebug>
#include <QMimeData>
#include <QCoreApplication>
#include <QMimeDatabase>
#include <QUrl>
#include <QFileInfo>
#include <QMenu>
#include <QAction>
#include <QDir>

#define CURRENTFILE "currentFile"
#define CURRENTFILES "currentFiles"
#define CURRENTDir "currentDir"

#define ORGANIZATION_NAME "MediaTools"
#define APPLICATION_NAME "MediaAnalyzer"
#define APPLICATION_VERSION "1.0.0"

constexpr auto LOG_SETTINGS_GROUP = "Log";
constexpr auto GENERAL_SETTINGS_GROUP = "General";
constexpr auto MAINWINDOW_SETTINGS_GROUP = "MainWindow";
constexpr auto WINDOWHEAD_SETTINGS_GROUP = "WindowHead";
constexpr auto RECENTFILES_SETTINGS_GROUP = "RecentFiles";
constexpr auto CONFIGURE_BUILDER_SETTINGS_GROUP = "ConfigureBuilder";

extern const QStringList CONFIG_GROUPS;

/**
 * @brief Get the translated display name for a config group key
 * @param groupKey The internal group key (e.g. "Log", "General")
 * @return Translated display name suitable for UI
 */
QString translatedConfigGroupName(const QString &groupKey);

/**
 * @brief Get the list of config groups with translated display names
 * @return Map of internal key → translated display name
 */
QMap<QString, QString> translatedConfigGroups();

constexpr auto CONFIGURE_BUILDER_RECENTFOLDERS_KEY = "RecentFolders";

// Image preview settings
constexpr auto IMAGE_PREVIEW_PATH_KEY = "ImagePreviewPath";
constexpr auto DEFAULT_IMAGE_PREVIEW_PATH = "/tmp/media_analyzer_preview_images";

// config
/**
 * @brief Macro definitions and default values for log configuration
 */
namespace LoggerConfig {
// Configuration key names
constexpr auto ENABLED_KEY = "enabled";
constexpr auto LEVEL_KEY = "logLevel";
constexpr auto DIRECTORY_KEY = "logDirectory";
constexpr auto MAX_FILE_SIZE_KEY = "maxFileSizeMB";
constexpr auto MAX_FILES_KEY = "maxFiles";
constexpr auto CAPTURE_QT_MESSAGES_KEY = "captureQtMessages";
constexpr auto FILE_NAME_PATTERN_KEY = "fileNamePattern";
constexpr auto LOG_FORMAT_KEY = "logFormat";
constexpr auto ENABLED_FILE = "enable_file";
constexpr auto ENABLED_LINE = "enable_line";
constexpr auto ENABLED_FUNCTION = "enable_function";

// Default values
constexpr bool DEFAULT_ENABLED = true;
constexpr int DEFAULT_LOG_LEVEL = 1; // INFO level
constexpr auto DEFAULT_DIRECTORY = ""; // Will be set to user's cache directory at runtime
constexpr int DEFAULT_MAX_FILE_SIZE = 10; // MB
constexpr int DEFAULT_MAX_FILES = 7;
constexpr bool DEFAULT_CAPTURE_QT_MESSAGES = true;
constexpr auto DEFAULT_FILE_NAME_PATTERN = "app_%1.log"; // %1 will be replaced by date
constexpr auto DEFAULT_LOG_FORMAT = "[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] [%{file}:%{line}] %{message}";
} // namespace LoggerConfig

typedef struct ZExtraInfo
{
    ZExtraInfo(QString command, QString format) {
        commandKey = command;
        formatKey = format;
    }

    ZExtraInfo(){}

    QString commandKey = "";
    QString formatKey = "json";
    QStringList commandList;
} ZExtraInfo;

Q_DECLARE_METATYPE(ZExtraInfo)

class Common
{
    // Q_OBJECT
public:
    Common(const Common&) = delete;
    Common& operator=(const Common&) = delete;

    static Common* instance();
    static void destroy();

    void setConfigValue(const QString& key, const QVariant &value);
    QVariant getConfigValue(const QString& key, const QVariant &defaultValue = QString()) const;

    void initialize();
    bool isInitialized() const;

    static QList<QStringList> logLevels;

    // Check if the file is a supported video format
    static bool isSupportedVideoFile(const QString &filePath);

    // Check if the file is a video (generic method)
    static bool isVideoFile(const QString &filePath);

    // Check if the file is an audio file
    static bool isAudioFile(const QString &filePath);

    // Check if the file is a media file (video or audio)
    static bool isMediaFile(const QString &filePath);

    // Check if the file is an image file
    static bool isImageFile(const QString &filePath);

    // Check if the file is a subtitle file
    static bool isSubtitleFile(const QString &filePath);

    // Check if the file is a supported media file (video, audio, image, subtitle)
    static bool isSupportedMediaFile(const QString &filePath);

    // Get all supported video MIME types
    static QSet<QString> supportedVideoMimeTypes();

    // Get all supported video file extensions
    static QSet<QString> supportedVideoExtensions();

    // Get all supported audio MIME types
    static QSet<QString> supportedAudioMimeTypes();

    // Get all supported audio file extensions
    static QSet<QString> supportedAudioExtensions();

    // Get all supported image MIME types
    static QSet<QString> supportedImageMimeTypes();

    // Get all supported image file extensions
    static QSet<QString> supportedImageExtensions();

    // Get all supported subtitle MIME types
    static QSet<QString> supportedSubtitleMimeTypes();

    // Get all supported subtitle file extensions
    static QSet<QString> supportedSubtitleExtensions();

    // Extract supported media file paths from MIME data
    static QStringList extractSupportedMediaFiles(const QMimeData *mimeData);

    // Check if MIME data contains supported media files
    static bool containsSupportedMediaFiles(const QMimeData *mimeData);

    // Menu utility functions
    static QAction *findActionByObjectName(QMenu *menu, const QString &objectName);
    static QAction *findActionByText(QMenu *menu, const QString &text);
    static QList<QAction *> getAllActions(QMenu *menu);

    // Directory management utility functions
    static bool ensureDirectory(const QString& dirPath);
    static void cleanupDirectory(const QString& dirPath);

    // Package version utility functions
    static QString getPackageVersion(const QString& packageName);
    static QString getCurrentPackageVersion();

private:
    Common();
    ~Common();

    bool m_initialized;
    static QMutex m_mutex;
    static Common* m_instance;

    // Initialize supported MIME types (lazy initialization)
    static const QSet<QString> &getSupportedVideoMimeTypes();

    // Initialize supported file extensions (lazy initialization)
    static const QSet<QString> &getSupportedVideoExtensions();

    // Initialize supported audio MIME types (lazy initialization)
    static const QSet<QString> &getSupportedAudioMimeTypes();

    // Initialize supported audio file extensions (lazy initialization)
    static const QSet<QString> &getSupportedAudioExtensions();

    // Initialize supported image MIME types (lazy initialization)
    static const QSet<QString> &getSupportedImageMimeTypes();

    // Initialize supported image file extensions (lazy initialization)
    static const QSet<QString> &getSupportedImageExtensions();

    // Initialize supported subtitle MIME types (lazy initialization)
    static const QSet<QString> &getSupportedSubtitleMimeTypes();

    // Initialize supported subtitle file extensions (lazy initialization)
    static const QSet<QString> &getSupportedSubtitleExtensions();
};

#endif // COMMON_H
