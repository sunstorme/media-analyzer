// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import "qrc:/qml/styles" as Styles

CheckBox {
    id: root
    
    // 属性
    property color normalBorderColor: Styles.Style.borderColor
    property color activeBorderColor: Styles.Style.primaryColor
    property color checkedColor: Styles.Style.primaryColor
    property color backgroundColor: Styles.Style.backgroundColor
    property int borderWidth: Styles.Style.borderWidth
    property int activeBorderWidth: Styles.Style.activeBorderWidth
    property color textColor: Styles.Style.textColor
    
    // 字体
    font: Styles.Style.bodyFont
    
    // 指示器大小
    implicitHeight: 24
    implicitWidth: 24
    
    // 内容间距
    spacing: Styles.Style.spacing + 2
    
    // 自定义指示器
    indicator: Rectangle {
        id: indicatorRect
        
        // 尺寸和位置
        width: Styles.Style.iconSize
        height: Styles.Style.iconSize
        anchors.verticalCenter: parent.verticalCenter
        x: root.text ? (root.mirrored ? root.width - width - root.rightPadding : root.leftPadding) : root.leftPadding + (root.availableWidth - width) / 2
        y: root.topPadding + (root.availableHeight - height) / 2
        
        // 外观
        color: root.checked ? root.checkedColor : root.backgroundColor
        border.color: root.activeFocus ? root.activeBorderColor : root.normalBorderColor
        border.width: root.activeFocus ? root.activeBorderWidth : root.borderWidth
        radius: Styles.Style.smallBorderRadius
        
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
        
        Behavior on color {
            ColorAnimation {
                duration: Styles.Style.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
        
        // 选中标记
        Rectangle {
            id: checkMark
            width: Styles.Style.iconSize / 4
            height: Styles.Style.iconSize / 4
            anchors.centerIn: parent
            color: "white"
            radius: 1
            opacity: root.checked ? 1 : 0
            scale: root.checked ? 1 : 0.5
            
            Behavior on opacity {
                NumberAnimation {
                    duration: Styles.Style.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }
            
            Behavior on scale {
                NumberAnimation {
                    duration: Styles.Style.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
    
    // 文本颜色
    contentItem: Text {
        text: root.text
        font: root.font
        color: root.textColor
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.indicator.width + root.spacing
    }
}
