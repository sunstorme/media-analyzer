// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef QTCOMPAT_H
#define QTCOMPAT_H

#include <QtGlobal>
#include <QTextStream>

// Compatibility macros for Qt5/Qt6 differences

// QString::SkipEmptyParts was moved to Qt::SkipEmptyParts in Qt6
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    #define QT_SKIP_EMPTY_PARTS QString::SkipEmptyParts
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #define QT_SKIP_EMPTY_PARTS Qt::SplitBehaviorFlags(Qt::SkipEmptyParts)
#else
    #define QT_SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#endif

// QTextStream::setCodec was removed in Qt6, UTF-8 is default
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #define QT_SET_TEXT_STREAM_CODEC(stream, codec) stream->setCodec(codec)
#else
    #define QT_SET_TEXT_STREAM_CODEC(stream, codec) /* UTF-8 is default in Qt6 */
#endif

// qSetMessagePattern is available in both Qt5 and Qt6
#define QT_SET_MESSAGE_PATTERN(pattern) qSetMessagePattern(pattern)

// QSortFilterProxyModel::setFilterRegExp was replaced with setFilterRegularExpression in Qt6
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    #define QT_SET_FILTER_REGEXP(proxyModel, pattern) proxyModel->setFilterRegExp(pattern)
#else
    #define QT_SET_FILTER_REGEXP(proxyModel, pattern) proxyModel->setFilterRegularExpression(pattern)
#endif

#endif // QTCOMPAT_H
