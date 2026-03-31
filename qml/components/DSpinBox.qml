// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import "qrc:/qml/styles" as Styles

SpinBox {
    id: root

    // 属性
    property color textColor: Styles.Style.textColor

    // 自定义信号：用户修改值时触发
    signal userModified()

    // 字体
    font: Styles.Style.bodyFont

    // 尺寸
    implicitHeight: Styles.Style.itemHeight
    implicitWidth: 120

    // 背景和边框
    background: DBackground {
        isFocused: root.activeFocus
    }

    // 内容项（文本输入框）
    contentItem: TextInput {
        id: textInput

        // 文本属性
        text: root.textFromValue(root.value, root.locale)
        font: root.font
        color: root.textColor
        selectionColor: Styles.Style.primaryColor
        selectedTextColor: "white"
        horizontalAlignment: Qt.AlignLeft
        verticalAlignment: Qt.AlignVCenter
        leftPadding: Styles.Style.padding
        rightPadding: root.height + Styles.Style.padding  // 为上下按钮留出空间

        // 验证器
        validator: root.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly

        // 值变化时更新文本
        onTextEdited: {
            var newValue = root.valueFromText(text, root.locale)
            if (newValue >= root.from && newValue <= root.to) {
                var oldValue = root.value
                root.value = newValue
                if (oldValue !== newValue) {
                    root.userModified()
                }
            }
        }

        // 失去焦点时也触发保存
        onEditingFinished: {
            root.userModified()
        }
    }

    // 向上按钮
    up.indicator: Item {
        x: root.mirrored ? 0 : parent.width - width
        height: parent.height / 2
        width: parent.height

        Text {
            text: "▲"
            font.pixelSize: Styles.Style.smallFont.pixelSize
            color: upMouseArea.containsMouse ? Styles.Style.primaryColor : root.textColor
            anchors.centerIn: parent

            Behavior on color {
                ColorAnimation {
                    duration: Styles.Style.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        MouseArea {
            id: upMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                root.increase()
                root.userModified()
            }
        }
    }

    // 向下按钮
    down.indicator: Item {
        x: root.mirrored ? 0 : parent.width - width
        y: parent.height / 2
        height: parent.height / 2
        width: parent.height

        Text {
            text: "▼"
            font.pixelSize: Styles.Style.smallFont.pixelSize
            color: downMouseArea.containsMouse ? Styles.Style.primaryColor : root.textColor
            anchors.centerIn: parent

            Behavior on color {
                ColorAnimation {
                    duration: Styles.Style.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        MouseArea {
            id: downMouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                root.decrease()
                root.userModified()
            }
        }
    }
}
