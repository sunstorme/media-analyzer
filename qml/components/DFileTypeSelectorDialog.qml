// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DFMMenu 1.0
import "qrc:/qml/styles" as Styles

Dialog {
    id: root
    title: qsTr("选择文件类型")
    modal: true
    width: 800
    height: 600
    
    // 居中显示
    anchors.centerIn: parent
    
    // 属性
    property var selectedSuffixes: []  // 已选择的后缀列表
    property var allFileTypes: fileTypeManager.allFileTypes || []
    property var categories: fileTypeManager.categories || {}
    
    // 信号
    signal selectionChanged(var suffixes)
    
    // 内部状态
    property string searchText: ""
    property var selectedCategories: []  // 已选择的类别
    property var selectionFilter: []      // 选择状态筛选：[]表示全部，["selected"]表示已选，["unselected"]表示未选
    property bool addPanelVisible: false  // 添加面板是否可见
    
    // 过滤后的文件类型列表
    property var filteredFileTypes: fileTypeManager.filterFileTypes(searchText, selectedCategories, selectionFilter)
    
    // 获取类别名称
    function getCategoryName(categoryKey) {
        return fileTypeManager.getCategoryName(categoryKey)
    }
    
    // 切换选择状态
    function toggleSelection(suffix) {
        fileTypeManager.toggleSelection(suffix)
        selectedSuffixes = fileTypeManager.getSelectedSuffixes()
        selectionChanged(selectedSuffixes)
        fileTypesModel.updateModel()
    }
    
    // 添加自定义文件类型
    function addCustomFileType(suffix, name, category) {
        if (!suffix || suffix.trim() === "") {
            return
        }
        
        suffix = suffix.trim().toLowerCase().replace(/^\./, "")
        name = name || suffix.toUpperCase()
        category = category || "other"
        
        // 调用C++方法添加
        fileTypeManager.addCustomFileType(suffix, name, category)
        
        // 更新已选择的后缀列表
        selectedSuffixes = fileTypeManager.getSelectedSuffixes()
        selectionChanged(selectedSuffixes)
        fileTypesModel.updateModel()
    }
    
    background: Rectangle {
        color: Styles.Style.backgroundColor
        border.color: Styles.Style.borderColor
        border.width: 1
        radius: Styles.Style.borderRadius
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: Styles.Style.spacing
        
        // 搜索和工具栏
        Row {
            Layout.fillWidth: true
            Layout.preferredHeight: Styles.Style.itemHeight
            spacing: Styles.Style.spacing
            
            // 搜索框
            TextField {
                id: searchField
                width: parent.width - toggleAddButton.width - parent.spacing
                height: parent.height
                placeholderText: qsTr("搜索文件类型...")
                font: Styles.Style.bodyFont
                
                onTextChanged: {
                    searchText = text
                }
                
                background: Rectangle {
                    color: Styles.Style.backgroundColor
                    border.color: Styles.Style.borderColor
                    border.width: 1
                    radius: Styles.Style.borderRadius
                }
            }
            
            // 折叠/展开添加面板按钮
            DButton {
                id: toggleAddButton
                width: Styles.Style.itemHeight * 2
                height: parent.height
                text: addPanelVisible ? "▼ " + qsTr("收起") : "▶ " + qsTr("添加")
                
                onClicked: {
                    addPanelVisible = !addPanelVisible
                }
            }
        }
        
        // 添加自定义文件类型面板
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: addPanelVisible ? implicitHeight : 0
            visible: addPanelVisible
            color: "transparent"
            border.color: "transparent"
            border.width: 1
            radius: Styles.Style.borderRadius
            
            Behavior on Layout.preferredHeight {
                NumberAnimation {
                    duration: Styles.Style.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }
            
            implicitHeight: addColumn.implicitHeight
            
            Column {
                id: addColumn
                anchors.top: parent.top
                anchors.fill: parent
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                // anchors.margins: Styles.Style.padding
                spacing: Styles.Style.spacing
                
                Row {
                    width: parent.width
                    height: Styles.Style.itemHeight
                    spacing: Styles.Style.spacing
                    
                    TextField {
                        id: newSuffixField
                        width: 150
                        height: parent.height
                        placeholderText: qsTr("后缀（如：xyz）")
                        font: Styles.Style.bodyFont
                        
                        background: Rectangle {
                            color: Styles.Style.backgroundColor
                            border.color: Styles.Style.borderColor
                            border.width: 1
                            radius: Styles.Style.borderRadius
                        }
                    }
                    
                    TextField {
                        id: newNameField
                        width: 200
                        height: parent.height
                        placeholderText: qsTr("类型名称（如：XYZ文件）")
                        font: Styles.Style.bodyFont
                        
                        background: Rectangle {
                            color: Styles.Style.backgroundColor
                            border.color: Styles.Style.borderColor
                            border.width: 1
                            radius: Styles.Style.borderRadius
                        }
                    }
                    
                    ComboBox {
                        id: newCategoryCombo
                        width: 150
                        height: parent.height
                        font: Styles.Style.bodyFont
                        model: ListModel {
                            id: categoryModel
                            Component.onCompleted: {
                                for (var key in categories) {
                                    append({text: categories[key].name, value: key})
                                }
                            }
                        }
                        textRole: "text"
                        valueRole: "value"
                        
                        background: Rectangle {
                            color: Styles.Style.backgroundColor
                            border.color: Styles.Style.borderColor
                            border.width: 1
                            radius: Styles.Style.borderRadius
                        }
                    }
                    
                    DButton {
                        width: 80
                        height: parent.height
                        text: qsTr("添加")
                        
                        onClicked: {
                            addCustomFileType(
                                newSuffixField.text,
                                newNameField.text,
                                newCategoryCombo.currentValue
                            )
                            newSuffixField.text = ""
                            newNameField.text = ""
                        }
                    }
                }
            }
        }
        
        // 筛选工具栏
        Rectangle {
            id: filterToolbar
            Layout.fillWidth: true
            
            color: Styles.Style.backgroundColor
            border.color: Styles.Style.borderColor
            border.width: 1
            radius: Styles.Style.borderRadius

            implicitHeight: filterRowLayout.implicitHeight + Styles.Style.padding
            
            RowLayout {
                id: filterRowLayout
                anchors.fill: parent
                anchors.margins: Styles.Style.padding / 2
                spacing: Styles.Style.spacing
                
                DMultiSelectComboBox {
                    id: statusFilterCombo
                    Layout.preferredWidth: 100
                    Layout.fillHeight: true
                    border.width: 0
                    
                    property var statusItems: [
                        {text: qsTr("已选"), value: "selected"},
                        {text: qsTr("未选"), value: "unselected"}
                    ]
                    
                    allItems: statusItems
                    displayRole: "text"
                    
                    onSelectionChanged: function(items) {
                        var values = items.map(function(item) { return item.value })
                        // 如果同时选中了"已选"和"未选"，则不进行状态筛选（显示所有）
                        if (values.indexOf("selected") >= 0 && values.indexOf("unselected") >= 0) {
                            selectionFilter = []
                        } else {
                            selectionFilter = values
                        }
                    }
                }

                Text {
                    Layout.fillHeight:true
                    Layout.preferredWidth: 150
                    Layout.alignment: Qt.AlignCenter
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("文件类型")
                }

                DMultiSelectComboBox {
                    id: categoryFilterCombo
                    Layout.preferredWidth: 150
                    Layout.leftMargin: 80
                    Layout.fillHeight: true
                    border.width: 0

                    property var categoryItems: {
                        var items = []
                        for (var key in categories) {
                            items.push({text: categories[key].name, value: key})
                        }
                        return items
                    }
                    
                    allItems: categoryItems
                    displayRole: "text"
                    
                    onSelectionChanged: function(items) {
                        selectedCategories = items.map(function(item) { return item.value })
                    }
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }
        
        // 文件类型列表
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Styles.Style.backgroundColor
            border.color: Styles.Style.borderColor
            border.width: 1
            radius: Styles.Style.borderRadius
            
            ScrollView {
                anchors.fill: parent
                anchors.margins: 1
                
                ListView {
                    id: fileTypesList
                    anchors.fill: parent
                    model: fileTypesModel
                    clip: true
                    spacing: 1
                    
                    delegate: Rectangle {
                        width: fileTypesList.width
                        height: Styles.Style.itemHeight
                        color: {
                            if (selectedSuffixes.indexOf(model.suffix) >= 0) {
                                return Styles.Style.selectColor
                            } else if (mouseArea.containsMouse) {
                                return Styles.Style.hoverColor
                            } else {
                                return "transparent"
                            }
                        }
                        
                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: Styles.Style.padding
                            anchors.rightMargin: Styles.Style.padding
                            spacing: Styles.Style.spacing
                            
                            // 选择checkbox
                            CheckBox {
                                id: selectCheckBox
                                anchors.verticalCenter: parent.verticalCenter
                                checked: selectedSuffixes.indexOf(model.suffix) >= 0
                                
                                onClicked: {
                                    toggleSelection(model.suffix)
                                }
                            }
                            
                            // 后缀
                            Text {
                                width: 100
                                anchors.verticalCenter: parent.verticalCenter
                                text: "." + model.suffix
                                font: Styles.Style.bodyFont
                                color: Styles.Style.textColor
                                elide: Text.ElideRight
                            }
                            
                            // 类型名称
                            Text {
                                width: 200
                                anchors.verticalCenter: parent.verticalCenter
                                text: model.name
                                font: Styles.Style.bodyFont
                                color: Styles.Style.textColor
                                elide: Text.ElideRight
                            }
                            
                            // 类别
                            Rectangle {
                                width: 100
                                height: Styles.Style.itemHeight - 10
                                anchors.verticalCenter: parent.verticalCenter
                                color: "transparent"
                                border.color: "transparent"
                                border.width: 1
                                radius: Styles.Style.borderRadius
                                
                                Text {
                                    anchors.centerIn: parent
                                    text: getCategoryName(model.category)
                                    font: Styles.Style.bodyFont
                                    color: Styles.Style.textColor
                                }
                            }
                            
                            Item {
                                width: parent.width - selectCheckBox.width - 100 - 200 - 100 - parent.spacing * 4 - Styles.Style.padding * 2
                                height: 1
                            }
                        }
                        
                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                toggleSelection(model.suffix)
                            }
                        }
                    }
                }
            }
        }
        
        // 按钮区域
        Row {
            id: buttonRow
            Layout.fillWidth: true
            Layout.preferredHeight: Styles.Style.itemHeight
            spacing: Styles.Style.spacing
            
            Item {
                width: parent.width - cancelButton.width - okButton.width - parent.spacing
                height: 1
            }
            
            DButton {
                id: cancelButton
                anchors.right: parent.right
                height: parent.height
                text: qsTr("取消")
                
                onClicked: {
                    root.reject()
                }
            }
            
            DButton {
                id: okButton
                anchors.right: cancelButton.left
                anchors.rightMargin: Styles.Style.spacing
                height: parent.height
                text: qsTr("确定")
                
                onClicked: {
                    root.accept()
                }
            }
        }
    }
    
    // 文件类型列表模型
    ListModel {
        id: fileTypesModel
        
        function updateModel() {
            clear()
            for (var i = 0; i < filteredFileTypes.length; i++) {
                var fileType = filteredFileTypes[i]
                append({
                    suffix: fileType.suffix,
                    name: fileType.name,
                    name_en: fileType.name_en,
                    category: fileType.category,
                    isCustom: fileType.isCustom || false
                })
            }
        }
        
        Component.onCompleted: {
            updateModel()
        }
    }
    
    Connections {
        target: root
        function onFilteredFileTypesChanged() {
            fileTypesModel.updateModel()
        }
    }
    
    onAccepted: {
        // 确认选择
        console.log("Selected suffixes:", selectedSuffixes.join(":"))
    }
    
    onRejected: {
        // 取消选择
        console.log("Dialog cancelled")
    }
}

