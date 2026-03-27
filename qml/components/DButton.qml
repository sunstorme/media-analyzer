// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import "qrc:/qml/styles" as Styles

Button {
    id: root
    
    // 属性
    property color normalColor: Styles.Style.backgroundColor
    property color hoverColor: Styles.Style.hoverColor
    property color pressedColor: Styles.Style.selectColor
    property color borderColor: Styles.Style.borderColor
    property int borderWidth: 1
    property color textColor: Styles.Style.textColor
    
    // 背景色
    property color backgroundColor: {
        if (pressed) {
            return pressedColor
        } else if (hovered) {
            return hoverColor
        } else {
            return normalColor
        }
    }
    
    // 字体
    font: Styles.Style.bodyFont
    width: 80
    
    // 背景矩形
    background: Rectangle {
        color: root.backgroundColor
        border.color: root.borderColor
        border.width: root.borderWidth
        radius: Styles.Style.borderRadius
        
        Behavior on color {
            ColorAnimation {
                duration: Styles.Style.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
    
    // 内容项
    contentItem: Text {
        text: root.text
        font: root.font
        color: root.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
