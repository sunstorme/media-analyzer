// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "file_type_manager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

FileTypeManager::FileTypeManager(QObject *parent)
    : QObject(parent)
{
    loadFileTypesData();
}

void FileTypeManager::loadFileTypesData()
{
    // 从资源文件加载
    QFile file(":/file_types.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file_types.json:" << file.errorString();
        return;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse file_types.json:" << error.errorString();
        return;
    }
    
    QJsonObject root = doc.object();
    
    // 加载类别
    QJsonObject categoriesObj = root["categories"].toObject();
    for (auto it = categoriesObj.begin(); it != categoriesObj.end(); ++it) {
        QVariantMap category;
        category["name"] = it.value().toObject()["name"].toString();
        category["name_en"] = it.value().toObject()["name_en"].toString();
        category["icon"] = it.value().toObject()["icon"].toString();
        m_categories[it.key()] = category;
    }
    
    // 加载文件类型
    QJsonArray fileTypesArray = root["file_types"].toArray();
    for (const QJsonValue &value : fileTypesArray) {
        QJsonObject typeObj = value.toObject();
        QVariantMap fileType;
        fileType["suffix"] = typeObj["suffix"].toString();
        fileType["name"] = typeObj["name"].toString();
        fileType["name_en"] = typeObj["name_en"].toString();
        fileType["category"] = typeObj["category"].toString();
        fileType["isCustom"] = false;
        m_allFileTypes.append(fileType);
    }
    
    // 加载自定义文件类型
    loadCustomFileTypes();
    
    qDebug() << "Loaded" << m_allFileTypes.size() << "file types";
}

void FileTypeManager::loadCustomFileTypes()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    
    QFile customFile(configPath + "/custom_file_types.json");
    if (!customFile.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(customFile.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse custom_file_types.json:" << error.errorString();
        return;
    }
    
    QJsonArray customTypes = doc.array();
    for (const QJsonValue &value : customTypes) {
        QJsonObject typeObj = value.toObject();
        QVariantMap fileType;
        fileType["suffix"] = typeObj["suffix"].toString();
        fileType["name"] = typeObj["name"].toString();
        fileType["name_en"] = typeObj["name_en"].toString();
        fileType["category"] = typeObj["category"].toString();
        fileType["isCustom"] = true;
        m_allFileTypes.append(fileType);
    }
    
    qDebug() << "Loaded" << customTypes.size() << "custom file types";
}

void FileTypeManager::saveCustomFileTypes()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    
    QJsonArray customTypes;
    for (const QVariant &typeVar : m_allFileTypes) {
        QVariantMap type = typeVar.toMap();
        if (type["isCustom"].toBool()) {
            QJsonObject typeObj;
            typeObj["suffix"] = type["suffix"].toString();
            typeObj["name"] = type["name"].toString();
            typeObj["name_en"] = type["name_en"].toString();
            typeObj["category"] = type["category"].toString();
            customTypes.append(typeObj);
        }
    }
    
    QJsonDocument doc(customTypes);
    QFile customFile(configPath + "/custom_file_types.json");
    if (!customFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save custom_file_types.json:" << customFile.errorString();
        return;
    }
    
    customFile.write(doc.toJson());
    qDebug() << "Saved" << customTypes.size() << "custom file types";
}

QStringList FileTypeManager::getSelectedSuffixes() const
{
    return m_selectedSuffixes;
}

void FileTypeManager::setSelectedSuffixes(const QStringList &suffixes)
{
    if (m_selectedSuffixes != suffixes) {
        m_selectedSuffixes = suffixes;
        emit selectionChanged(m_selectedSuffixes);
    }
}

void FileTypeManager::toggleSelection(const QString &suffix)
{
    int index = m_selectedSuffixes.indexOf(suffix);
    if (index >= 0) {
        m_selectedSuffixes.removeAt(index);
    } else {
        m_selectedSuffixes.append(suffix);
    }
    emit selectionChanged(m_selectedSuffixes);
}

bool FileTypeManager::isSelected(const QString &suffix) const
{
    return m_selectedSuffixes.contains(suffix);
}

void FileTypeManager::addCustomFileType(const QString &suffix, const QString &name, const QString &category)
{
    QString cleanSuffix = suffix.trimmed().toLower();
    if (cleanSuffix.startsWith('.')) {
        cleanSuffix = cleanSuffix.mid(1);
    }
    
    if (cleanSuffix.isEmpty()) {
        return;
    }
    
    // 检查是否已存在
    for (const QVariant &typeVar : m_allFileTypes) {
        QVariantMap type = typeVar.toMap();
        if (type["suffix"].toString() == cleanSuffix) {
            return;
        }
    }
    
    // 添加新类型
    QVariantMap newType;
    newType["suffix"] = cleanSuffix;
    newType["name"] = name.isEmpty() ? cleanSuffix.toUpper() : name;
    newType["name_en"] = name.isEmpty() ? cleanSuffix.toUpper() : name;
    newType["category"] = category.isEmpty() ? "other" : category;
    newType["isCustom"] = true;
    m_allFileTypes.append(newType);
    
    // 自动选中新添加的类型
    if (!m_selectedSuffixes.contains(cleanSuffix)) {
        m_selectedSuffixes.append(cleanSuffix);
        emit selectionChanged(m_selectedSuffixes);
    }
    
    // 保存到文件
    saveCustomFileTypes();
    emit fileTypesChanged();
    
    qDebug() << "Added custom file type:" << cleanSuffix << name << category;
}

QVariantList FileTypeManager::filterFileTypes(const QString &searchText,
                                              const QStringList &categories,
                                              const QStringList &selectionFilter) const
{
    QVariantList result;
    
    for (const QVariant &typeVar : m_allFileTypes) {
        QVariantMap type = typeVar.toMap();
        QString suffix = type["suffix"].toString();
        QString name = type["name"].toString();
        QString nameEn = type["name_en"].toString();
        QString category = type["category"].toString();
        
        // 搜索过滤
        if (!searchText.isEmpty()) {
            QString searchLower = searchText.toLower();
            if (!suffix.contains(searchLower) &&
                !name.toLower().contains(searchLower) &&
                !nameEn.toLower().contains(searchLower)) {
                continue;
            }
        }
        
        // 类别过滤
        if (!categories.isEmpty()) {
            if (!categories.contains(category)) {
                continue;
            }
        }
        
        // 选择状态过滤
        bool isSelected = m_selectedSuffixes.contains(suffix);
        if (!selectionFilter.isEmpty()) {
            if (selectionFilter.contains("selected") && !isSelected) {
                continue;
            }
            if (selectionFilter.contains("unselected") && isSelected) {
                continue;
            }
        }
        
        result.append(type);
    }
    
    return result;
}

QString FileTypeManager::getCategoryName(const QString &categoryKey) const
{
    if (m_categories.contains(categoryKey)) {
        QVariantMap category = m_categories[categoryKey].toMap();
        return category["name"].toString();
    }
    return categoryKey;
}
