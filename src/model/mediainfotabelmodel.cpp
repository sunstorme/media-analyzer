// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "mediainfotabelmodel.h"

MediaInfoTabelModel::MediaInfoTabelModel(QObject *parent) : QAbstractTableModel(parent),
    row(0), column(0), m_header(nullptr), m_data(nullptr)
{

}

int MediaInfoTabelModel::rowCount(const QModelIndex &parent) const
{
    return row;
}

int MediaInfoTabelModel::columnCount(const QModelIndex &parent) const
{
    return column;
}

QVariant MediaInfoTabelModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (!m_data || !index.isValid() || index.row() >= m_data->size() || 
            index.column() >= m_data->at(index.row()).size()) {
            return QVariant();
        }
        return (*m_data)[index.row()][index.column()];
    }

    if(role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }

    // if(role == Qt::BackgroundRole &&  index.row() % 2 == 0)
    // {
    //     return QBrush(QColor(50, 50, 50));
    // }

    return QVariant();
}

QVariant MediaInfoTabelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if(m_header && section >= 0 && section < m_header->count())
            return m_header->at(section);
    }

    if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
        return QString::number(section + 1);
    }

    //qDebug()<<role << "--" << Qt::BackgroundRole;

    //    if(role == Qt::BackgroundRole)
    //    {
    //        return QBrush(QColor(156, 233, 248));
    //    }
    //    if(role == Qt::ForegroundRole)
    //    {
    //         return QBrush(QColor(156, 233, 248));
    //    }

    if(role == Qt::FontRole)
    {
        return QFont(tr("微软雅黑"),10, QFont::DemiBold);
    }
    return QVariant();
}

bool MediaInfoTabelModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        if (!checkIndex(index) || !m_data)
            return false;

        //save value from editor to member m_gridData
        (*m_data)[index.row()][index.column()] = value.toString();
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }
    return false;
}

Qt::ItemFlags MediaInfoTabelModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

void MediaInfoTabelModel::setRow(int newRow)
{
    row = newRow;
}

void MediaInfoTabelModel::setColumn(int newColumn)
{
    column = newColumn;
}

void MediaInfoTabelModel::setTableHeader(QList<QString> *header)
{
    beginResetModel();
    m_header = header;
    endResetModel();
}

void MediaInfoTabelModel::setTableData(QList<QStringList> *data)
{
    beginResetModel();
    m_data = data;
    endResetModel();
}

void MediaInfoTabelModel::appendRow(const QStringList &rowData)
{
    if (!m_data) return;
    int firstRow = row;
    beginInsertRows(QModelIndex(), firstRow, firstRow);
    m_data->append(rowData);
    row++;
    endInsertRows();
}

void MediaInfoTabelModel::appendRows(const QList<QStringList> &rows)
{
    if (!m_data || rows.isEmpty()) return;
    int firstRow = row;
    int lastRow = row + rows.size() - 1;
    beginInsertRows(QModelIndex(), firstRow, lastRow);
    for (const QStringList &rowData : rows) {
        m_data->append(rowData);
    }
    row += rows.size();
    endInsertRows();
}

void MediaInfoTabelModel::SlotUpdateTable()
{
    emit dataChanged(createIndex(0, 0), createIndex(row, column), {Qt::DisplayRole});
}
