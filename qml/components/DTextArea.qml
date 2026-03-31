// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import "qrc:/qml/styles" as Styles

TextArea {
    id: root

    // 属性
    property color textColor: Styles.Style.textColor

    // 字体
    font: Styles.Style.bodyFont

    // 颜色
    color: textColor

    // 确保失去焦点时不保持选中状态
    persistentSelection: false

    // 背景矩形
    background: DBackground {
        isFocused: root.activeFocus
    }

    // 选中文本颜色
    selectionColor: Styles.Style.primaryColor
}
