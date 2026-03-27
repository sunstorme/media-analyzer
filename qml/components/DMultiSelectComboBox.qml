// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "qrc:/qml/styles" as Styles

Rectangle {
    id: root
    
    // 属性
    property var selectedItems: []  // 已选择的项
    property var allItems: []       // 所有可选项
    property string displayRole: "text"  // 显示角色
    property alias text: displayText.text
    property int comboHeight: Styles.Style.itemHeight
    
    // 信号
    signal selectionChanged(var items)
    signal activated()
    
    // 默认尺寸
    implicitWidth: 150
    implicitHeight: comboHeight
    height: comboHeight
    
    // 样式
    color: mouseArea.containsMouse ? Styles.Style.hoverColor : Styles.Style.backgroundColor
    border.color: Styles.Style.borderColor
    border.width: 1
    radius: Styles.Style.borderRadius
    
    Behavior on color {
        ColorAnimation {
            duration: Styles.Style.animationDuration
            easing.type: Easing.InOutQuad
        }
    }
    
    // 显示文本
    Text {
        id: displayText
        anchors.left: parent.left
        anchors.leftMargin: Styles.Style.padding / 2
        anchors.right: arrowIcon.left
        anchors.rightMargin: Styles.Style.spacing / 2
        anchors.verticalCenter: parent.verticalCenter
        font: Styles.Style.bodyFont
        color: Styles.Style.textColor
        elide: Text.ElideRight
        
        text: {
            if (selectedItems.length === 0) {
                return qsTr("全部")
            } else if (selectedItems.length === 1) {
                return selectedItems[0][displayRole] || selectedItems[0].toString()
            } else {
                return qsTr("已选 %1 项").arg(selectedItems.length)
            }
        }
    }
    
    // 下拉箭头
    Text {
        id: arrowIcon
        anchors.right: parent.right
        anchors.rightMargin: Styles.Style.padding
        anchors.verticalCenter: parent.verticalCenter
        text: popup.visible ? "▲" : "▼"
        font: Styles.Style.tagFont
        color: Styles.Style.secondaryTextColor
    }
    
    // 鼠标区域
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            popup.open()
            activated()
        }
    }
    
    // 弹出面板
    Popup {
        id: popup
        width: root.width
        height: Math.min(300, contentItem.childrenRect.height + 10)
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        parent: root.parent  // 设置父对象为 root 的父对象，避免渲染问题
        
        background: Rectangle {
            color: Styles.Style.backgroundColor
            border.color: Styles.Style.borderColor
            border.width: 1
            radius: Styles.Style.borderRadius
        }
        
        // 设置弹出位置到 root 下方
        x: root.x
        y: root.y + root.height + 5
        
        contentItem: ListView {
            model: root.allItems
            clip: true
            spacing: 1
            
            // 确保 implicitHeight 正确计算
            implicitHeight: count * (Styles.Style.itemHeight + spacing)
            
            delegate: Rectangle {
                width: root.width
                height: Styles.Style.itemHeight
                color: {
                    if (isSelected) {
                        return Styles.Style.selectColor
                    } else if (itemMouseArea.containsMouse) {
                        return Styles.Style.hoverColor
                    } else {
                        return "transparent"
                    }
                }
                
                property bool isSelected: {
                    for (var i = 0; i < selectedItems.length; i++) {
                        if (selectedItems[i].value === modelData.value) {
                            return true
                        }
                    }
                    return false
                }
                
                Row {
                    anchors.fill: parent
                    anchors.leftMargin: Styles.Style.padding / 2
                    anchors.rightMargin: Styles.Style.padding / 2
                    spacing: Styles.Style.spacing
                    
                    CheckBox {
                        id: checkBox
                        anchors.verticalCenter: parent.verticalCenter
                        checked: parent.parent.isSelected
                        
                        onClicked: {
                            toggleSelection(modelData)
                        }
                    }
                    
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData[displayRole] || modelData.value
                        font: Styles.Style.bodyFont
                        color: Styles.Style.textColor
                    }
                    
                    Item {
                        width: parent.width - checkBox.width - parent.children[1].width - parent.spacing * 2 - Styles.Style.padding * 2
                        height: 1
                    }
                }
                
                MouseArea {
                    id: itemMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        toggleSelection(modelData)
                    }
                }
            }
        }
    }
    
    // 切换选择状态
    function toggleSelection(item) {
        var found = false
        var newSelected = []
        
        // 复制已选项
        for (var i = 0; i < selectedItems.length; i++) {
            if (selectedItems[i].value === item.value) {
                found = true
            } else {
                newSelected.push(selectedItems[i])
            }
        }
        
        // 如果未找到，添加到选择列表
        if (!found) {
            newSelected.push(item)
        }
        
        selectedItems = newSelected
        selectionChanged(selectedItems)
    }
    
    // 清空选择
    function clearSelection() {
        selectedItems = []
        selectionChanged(selectedItems)
    }
    
    // 全选
    function selectAll() {
        selectedItems = allItems.slice()
        selectionChanged(selectedItems)
    }
}
