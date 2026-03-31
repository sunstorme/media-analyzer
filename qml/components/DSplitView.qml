// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import "qrc:/qml/styles" as Styles

SplitView {
    id: root
    
    // Handle 属性
    property int handleNormalWidth: 1
    property int handleHoverWidth: 4
    property color handleNormalColor: Styles.Style.borderColor
    property color handleHoverColor: Styles.Style.primaryColor
    
    // 自定义 handle
    handle: Rectangle {
        id: handleItem
        
        // 尺寸
        width: root.orientation === Qt.Horizontal ? handleNormalWidth : parent.width
        height: root.orientation === Qt.Horizontal ? parent.height : handleNormalWidth
        
        // 外观
        color: handleNormalColor
        opacity: handleMouseArea.containsMouse ? 1.0 : 0.3
        
        // 动画
        Behavior on width {
            NumberAnimation {
                duration: Styles.Style.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
        
        Behavior on height {
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
        
        Behavior on opacity {
            NumberAnimation {
                duration: Styles.Style.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
        
        // 鼠标区域 - 用于检测悬停
        MouseArea {
            id: handleMouseArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton
            
            // 设置鼠标光标
            cursorShape: {
                if (root.orientation === Qt.Horizontal) {
                    return Qt.SplitHCursor
                } else {
                    return Qt.SplitVCursor
                }
            }
            
            // 扩大鼠标检测区域，使 handle 更容易悬停
            anchors.topMargin: root.orientation === Qt.Horizontal ? 0 : -handleHoverWidth
            anchors.bottomMargin: root.orientation === Qt.Horizontal ? 0 : -handleHoverWidth
            anchors.leftMargin: root.orientation === Qt.Horizontal ? -handleHoverWidth : 0
            anchors.rightMargin: root.orientation === Qt.Horizontal ? -handleHoverWidth : 0
            
            onContainsMouseChanged: {
                if (containsMouse) {
                    // 悬停时增大 handle
                    if (root.orientation === Qt.Horizontal) {
                        handleItem.width = handleHoverWidth
                    } else {
                        handleItem.height = handleHoverWidth
                    }
                    handleItem.color = handleHoverColor
                    handleItem.opacity = 1.0
                } else {
                    // 离开时恢复
                    if (root.orientation === Qt.Horizontal) {
                        handleItem.width = handleNormalWidth
                    } else {
                        handleItem.height = handleNormalWidth
                    }
                    handleItem.color = handleNormalColor
                    handleItem.opacity = 0.3
                }
            }
        }
    }
}
