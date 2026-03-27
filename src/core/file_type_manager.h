// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FILE_TYPE_MANAGER_H
#define FILE_TYPE_MANAGER_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QStringList>

class FileTypeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList allFileTypes READ allFileTypes CONSTANT)
    Q_PROPERTY(QVariantMap categories READ categories CONSTANT)
    
public:
    explicit FileTypeManager(QObject *parent = nullptr);
    
    QVariantList allFileTypes() const { return m_allFileTypes; }
    QVariantMap categories() const { return m_categories; }
    
    Q_INVOKABLE QStringList getSelectedSuffixes() const;
    Q_INVOKABLE void setSelectedSuffixes(const QStringList &suffixes);
    Q_INVOKABLE void toggleSelection(const QString &suffix);
    Q_INVOKABLE bool isSelected(const QString &suffix) const;
    Q_INVOKABLE void addCustomFileType(const QString &suffix, const QString &name, const QString &category);
    Q_INVOKABLE QVariantList filterFileTypes(const QString &searchText,
                                             const QStringList &categories,
                                             const QStringList &selectionFilter) const;
    Q_INVOKABLE QString getCategoryName(const QString &categoryKey) const;
    
signals:
    void selectionChanged(const QStringList &suffixes);
    void fileTypesChanged();
    
private:
    void loadFileTypesData();
    void loadCustomFileTypes();
    void saveCustomFileTypes();
    
    QVariantList m_allFileTypes;
    QVariantMap m_categories;
    QStringList m_selectedSuffixes;
};

#endif // FILE_TYPE_MANAGER_H
