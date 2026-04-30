// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "filetypeutils.h"
#include "common/common.h"

MediaCategory FileTypeUtils::detectCategory(const QString &filePath)
{
    if (Common::isVideoFile(filePath))
        return MediaCategory::Video;
    if (Common::isAudioFile(filePath))
        return MediaCategory::Audio;
    if (Common::isImageFile(filePath))
        return MediaCategory::Image;
    return MediaCategory::Unknown;
}

MediaCategory FileTypeUtils::detectCategory(const std::string &filePath)
{
    return detectCategory(QString::fromStdString(filePath));
}

bool FileTypeUtils::isMediaFile(const QString &filePath)
{
    return detectCategory(filePath) != MediaCategory::Unknown;
}
