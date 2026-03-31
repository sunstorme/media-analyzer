// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import "qrc:/qml/styles" as Styles

Rectangle {
    id: root

    // 属性
    property bool isFocused: parent.activeFocus || false

    // 外观
    color: Styles.Style.backgroundColor
    border.color: isFocused ? Styles.Style.primaryColor : Styles.Style.borderColor
    border.width: isFocused ? Styles.Style.activeBorderWidth : Styles.Style.borderWidth
    radius: Styles.Style.borderRadius

    // 动画
    Behavior on border.color {
        ColorAnimation {
            duration: Styles.Style.animationDuration
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on border.width {
        NumberAnimation {
            duration: Styles.Style.animationDuration
            easing.type: Easing.InOutQuad
        }
    }
}
