// SPDX-FileCopyrightText: 2026 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef FILETYPEUTILS_H
#define FILETYPEUTILS_H

#include <QString>
#include <string>

enum class MediaCategory { Video, Audio, Image, Unknown };

class FileTypeUtils
{
public:
    static MediaCategory detectCategory(const QString &filePath);
    static MediaCategory detectCategory(const std::string &filePath);
    static bool isMediaFile(const QString &filePath);
};

#endif // FILETYPEUTILS_H
