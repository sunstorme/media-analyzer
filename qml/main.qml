// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DFMMenu 1.0
import "qrc:/qml/styles" as Styles
import "qrc:/qml/components" as Components

ApplicationWindow {
    id: root
    visible: true
    width: 1400
    height: 900
    minimumWidth: 1000
    minimumHeight: 600
    title: qsTr("DFM Context Menu Manager")
    
    // TreeView delegate
    Component {
        id: treeViewDelegate
        
        Rectangle {
            id: delegateItem
            
            implicitWidth: userHeader.width
            implicitHeight: label.implicitHeight * 2
            
            readonly property real indent: 20
            readonly property real padding: 5
            property bool isEditing: false
            
            // 当 isEditing 改变时，更新全局 editingDelegate
            onIsEditingChanged: {
                if (isEditing) {
                    // 如果有其他项目正在编辑，先结束它
                    if (root.editingDelegate && root.editingDelegate !== delegateItem && root.editingDelegate.isEditing) {
                        root.editingDelegate.isEditing = false
                    }
                    root.editingDelegate = delegateItem
                } else if (root.editingDelegate === delegateItem) {
                    root.editingDelegate = null
                }
            }
            
            required property TreeView treeView
            required property bool isTreeNode
            required property bool expanded
            required property int hasChildren
            required property int depth
            
            color: "transparent"
            
            border.color: "transparent"
            border.width: 1
            radius: 4
            
            // 获取当前项的索引（使用ID）
            function getCurrentIndex() {
                return menuTreeView.model.getIndex(model.id || "")
            }
            
            // 右键菜单
            Menu {
                id: contextMenu
                
                Menu {
                    title: qsTr("Add")
                    
                    MenuItem {
                        text: qsTr("Add Sibling Menu")
                        onTriggered: {
                            console.log("Add sibling menu for:", model.name)
                            var index = getCurrentIndex()
                            menuTreeView.model.addSiblingItem(index, qsTr("New Menu"))
                            // 自动进入编辑模式
                            editTimer.start()
                        }
                    }
                    
                    MenuItem {
                        text: qsTr("Add Child Menu")
                        enabled: model.level < 3
                        onTriggered: {
                            console.log("Add child menu for:", model.name)
                            var index = getCurrentIndex()
                            menuTreeView.model.addChildItem(index, qsTr("New Menu"))
                            // 展开节点
                            treeView.expand(row)
                            // 自动进入编辑模式
                            editTimer.start()
                        }
                    }
                }
                
                MenuItem {
                    text: qsTr("Rename")
                    onTriggered: {
                        console.log("Rename menu:", model.name)
                        delegateItem.isEditing = true
                        editTextField.forceActiveFocus()
                        editTextField.selectAll()
                    }
                }
                
                MenuSeparator {}
                
                MenuItem {
                    text: qsTr("Delete")
                    enabled: !model.isSystem
                    onTriggered: {
                        console.log("Delete menu:", model.name)
                        var index = getCurrentIndex()
                        menuTreeView.model.removeItem(index)
                        // 保存到文件
                        menuManager.saveCurrentModel()
                    }
                }
            }
            
            // 定时器，用于延迟进入编辑模式（等待模型更新）
            Timer {
                id: editTimer
                interval: 100
                onTriggered: {
                    delegateItem.isEditing = true
                    editTextField.forceActiveFocus()
                    editTextField.selectAll()
                }
            }
            
            Rectangle {
                width: delegateItem.padding
                height: parent.height / 2
                anchors.verticalCenter: parent.verticalCenter 
                visible: !model.column && (model.row === delegateItem.treeView.currentRow)
                color: Styles.Style.primaryColor
            }
            
            // 展开/折叠按钮
            Text {
                id: indicator
                visible: delegateItem.isTreeNode && delegateItem.hasChildren > 0
                x: delegateItem.padding + (delegateItem.depth * delegateItem.indent)
                width: delegateItem.indent
                height: delegateItem.height
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: delegateItem.expanded ? "▼" : "▶"
                font: Styles.Style.tagFont
                color: Styles.Style.secondaryTextColor
                
                TapHandler {
                    onTapped: {
                        // 如果有其他项目正在编辑，先结束编辑
                        if (root.editingDelegate && root.editingDelegate !== delegateItem && root.editingDelegate.isEditing) {
                            if (root.editingDelegate.editTextField && root.editingDelegate.editTextField.text.trim() !== "") {
                                var editingIndex = root.editingDelegate.getCurrentIndex()
                                menuTreeView.model.renameItem(editingIndex, root.editingDelegate.editTextField.text.trim())
                                menuManager.saveCurrentModel()
                            }
                            root.editingDelegate.isEditing = false
                        }
                        treeView.toggleExpanded(row)
                        console.log("Toggle expand:", model.name, "expanded:", !delegateItem.expanded, "hasChildren:", delegateItem.hasChildren)
                    }
                }
            }
            
            // 文本内容（非编辑模式）
            Text {
                id: label
                visible: !delegateItem.isEditing
                x: delegateItem.padding + (delegateItem.isTreeNode ? (delegateItem.depth + 1) * delegateItem.indent : delegateItem.depth * delegateItem.indent)
                width: delegateItem.width - delegateItem.padding - x
                height: delegateItem.height
                verticalAlignment: Text.AlignVCenter
                text: model.nameLocal || model.name || ""
                font: Styles.Style.bodyFont
                color: Styles.Style.textColor
                elide: Text.ElideRight
                
                TapHandler {
                    onTapped: {
                        console.log("Clicked item:", model.name, "nameLocal:", model.nameLocal, "depth:", delegateItem.depth, "hasChildren:", delegateItem.hasChildren)
                        // 如果有其他项目正在编辑，先结束编辑
                        if (root.editingDelegate && root.editingDelegate !== delegateItem && root.editingDelegate.isEditing) {
                            if (root.editingDelegate.editTextField && root.editingDelegate.editTextField.text.trim() !== "") {
                                var editingIndex = root.editingDelegate.getCurrentIndex()
                                menuTreeView.model.renameItem(editingIndex, root.editingDelegate.editTextField.text.trim())
                                menuManager.saveCurrentModel()
                            }
                            root.editingDelegate.isEditing = false
                        }
                        // 更新当前选中的菜单项
                        currentItem = model
                    }
                    
                    onDoubleTapped: {
                        console.log("Double clicked item:", model.name)
                        if (!model.isSystem) {
                            delegateItem.isEditing = true
                            editTextField.forceActiveFocus()
                            editTextField.selectAll()
                        }
                    }
                }
            }
            
            // 编辑模式下的TextField
            TextField {
                id: editTextField
                visible: delegateItem.isEditing
                x: delegateItem.padding + (delegateItem.isTreeNode ? (delegateItem.depth + 1) * delegateItem.indent : delegateItem.depth * delegateItem.indent)
                width: delegateItem.width - delegateItem.padding - x - 10
                height: delegateItem.height - 4
                anchors.verticalCenter: parent.verticalCenter
                text: model.nameLocal || model.name || ""
                font: Styles.Style.bodyFont
                
                background: Rectangle {
                    color: Styles.Style.backgroundColor
                    border.color: Styles.Style.primaryColor
                    border.width: 1
                    radius: Styles.Style.borderRadius
                }
                
                onAccepted: {
                    console.log("Edit accepted, new name:", text)
                    if (text.trim() !== "") {
                        var index = getCurrentIndex()
                        menuTreeView.model.renameItem(index, text.trim())
                        // 保存到文件
                        menuManager.saveCurrentModel()
                    }
                    delegateItem.isEditing = false
                }
                
                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        console.log("Enter pressed, new name:", text)
                        if (text.trim() !== "") {
                            var index = getCurrentIndex()
                            menuTreeView.model.renameItem(index, text.trim())
                            // 保存到文件
                            menuManager.saveCurrentModel()
                        }
                        delegateItem.isEditing = false
                        event.accepted = true
                    } else if (event.key === Qt.Key_Escape) {
                        console.log("Escape pressed, cancel edit")
                        delegateItem.isEditing = false
                        event.accepted = true
                    }
                }
                
                onFocusChanged: {
                    if (!focus && delegateItem.isEditing) {
                        console.log("Focus lost, save edit")
                        if (text.trim() !== "") {
                            var index = getCurrentIndex()
                            menuTreeView.model.renameItem(index, text.trim())
                            // 保存到文件
                            menuManager.saveCurrentModel()
                        }
                        delegateItem.isEditing = false
                    }
                }
            }
            
            // 右键菜单触发器
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.RightButton
                propagateComposedEvents: true
                
                onClicked: function(mouse) {
                    // 如果有其他项目正在编辑，先结束编辑
                    if (root.editingDelegate && root.editingDelegate !== delegateItem && root.editingDelegate.isEditing) {
                        if (root.editingDelegate.editTextField && root.editingDelegate.editTextField.text.trim() !== "") {
                            var editingIndex = root.editingDelegate.getCurrentIndex()
                            menuTreeView.model.renameItem(editingIndex, root.editingDelegate.editTextField.text.trim())
                            menuManager.saveCurrentModel()
                        }
                        root.editingDelegate.isEditing = false
                    }
                    // 更新当前选中的菜单项
                    currentItem = model
                    // 显示右键菜单
                    contextMenu.popup(mouse)
                    mouse.accepted = true
                }
            }
        }
    }
    
    // 窗口状态管理
    Component.onCompleted: {
        WindowManager.restoreState(root, root)
    }
    
    Component.onDestruction: {
        WindowManager.saveState(root, filePanelWidth, menuEditorWidth, propertyPanelWidth)
    }
    
    // 主布局 - 使用SplitView实现可拖动分隔器
    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal
        
        // 左侧面板 - 文件管理区
        Rectangle {
            id: filePanel
            SplitView.minimumWidth: 250
            SplitView.preferredWidth: filePanelWidth
            color: Styles.Style.backgroundColor
            
            Column {
                anchors.fill: parent
                anchors.margins: Styles.Style.padding
                spacing: Styles.Style.spacing
                
                // 搜索框
                TextField {
                    id: searchBox
                    width: parent.width
                    height: Styles.Style.itemHeight
                    placeholderText: qsTr("Search configuration files...")
                    font: Styles.Style.bodyFont
                    
                    onTextChanged: {
                        userFileModel.searchFilter = text
                        systemFileModel.searchFilter = text
                    }
                    
                    background: Rectangle {
                        color: Styles.Style.backgroundColor
                        border.color: Styles.Style.borderColor
                        border.width: 1
                        radius: Styles.Style.borderRadius
                    }
                }
                
                ScrollView {
                    width: parent.width
                    height: parent.height - searchBox.height - Styles.Style.spacing
                    
                    ColumnLayout {
                        width: parent.width
                        spacing: Styles.Style.spacing
                        
                        // 用户配置文件区(可折叠)
                        Rectangle {
                            id: userGroupBox
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: Styles.Style.backgroundColor
                            border.color: Styles.Style.borderColor
                            border.width: 1
                            radius: Styles.Style.borderRadius
                            
                            implicitHeight: userHeader.height + (userExpanded ? userContent.height + Styles.Style.padding : 0)
                            
                            Behavior on implicitHeight {
                                NumberAnimation {
                                    duration: Styles.Style.animationDuration
                                    easing.type: Easing.InOutQuad
                                }
                            }
                            
                            // 标题栏
                            Rectangle {
                                id: userHeader
                                width: parent.width
                                height: Styles.Style.itemHeight
                                color: Styles.Style.hoverColor
                                radius: Styles.Style.borderRadius
                                
                                Text {
                                    anchors.left: parent.left
                                    anchors.leftMargin: Styles.Style.padding
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: qsTr("User Configuration")
                                    font: Styles.Style.h2Font
                                    color: Styles.Style.textColor
                                }
                                
                                Text {
                                    anchors.right: parent.right
                                    anchors.rightMargin: Styles.Style.padding
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: userExpanded ? "▼" : "▶"
                                    font: Styles.Style.tagFont
                                    color: Styles.Style.secondaryTextColor
                                }
                                
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        userExpanded = !userExpanded
                                    }
                                }
                            }
                            
                            // 内容区域
                            Item {
                                id: userContent
                                anchors.top: userHeader.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.topMargin: Styles.Style.padding
                                anchors.leftMargin: Styles.Style.padding
                                anchors.rightMargin: Styles.Style.padding
                                height: userExpanded ? implicitHeight : 0
                                implicitHeight: userList.implicitHeight
                                clip: true
                                visible: userExpanded
                                
                                ListView {
                                    id: userList
                                    width: parent.width
                                    height: parent.height
                                    implicitHeight: contentHeight
                                    model: MenuFileModel {
                                        id: userFileModel
                                        showSystemOnly: false
                                    }
                                    delegate: fileDelegate
                                    clip: true
                                    
                                    // 键盘事件处理
                                    Keys.onPressed: function(event) {
                                        console.log("userList: Key pressed:", event.key)
                                        if (event.key === Qt.Key_F5) {
                                            console.log("F5 pressed, refreshing file lists")
                                            userFileModel.refresh()
                                            systemFileModel.refresh()
                                            event.accepted = true
                                        } else if (event.key === Qt.Key_Delete) {
                                            console.log("Delete key pressed, selectedFilePath:", selectedFilePath)
                                            if (selectedFilePath !== "") {
                                                var isSystemFile = selectedFilePath.indexOf("/usr/share/") !== -1
                                                if (!isSystemFile) {
                                                    console.log("Deleting user file:", selectedFilePath)
                                                    userFileModel.deleteFile(selectedFilePath)
                                                } else {
                                                    console.log("Cannot delete system file")
                                                }
                                                event.accepted = true
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        
                        // 系统配置文件区(可折叠)
                        Rectangle {
                            id: systemGroupBox
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: Styles.Style.backgroundColor
                            border.color: Styles.Style.borderColor
                            border.width: 1
                            radius: Styles.Style.borderRadius
                            
                            implicitHeight: systemHeader.height + (systemExpanded ? systemContent.height + Styles.Style.padding : 0)
                            
                            Behavior on implicitHeight {
                                NumberAnimation {
                                    duration: Styles.Style.animationDuration
                                    easing.type: Easing.InOutQuad
                                }
                            }
                            
                            // 标题栏
                            Rectangle {
                                id: systemHeader
                                width: parent.width
                                height: Styles.Style.itemHeight
                                color: Styles.Style.hoverColor
                                radius: Styles.Style.borderRadius
                                
                                Text {
                                    anchors.left: parent.left
                                    anchors.leftMargin: Styles.Style.padding
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: qsTr("System Configuration")
                                    font: Styles.Style.h2Font
                                    color: Styles.Style.textColor
                                }
                                
                                Text {
                                    anchors.right: parent.right
                                    anchors.rightMargin: Styles.Style.padding
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: systemExpanded ? "▼" : "▶"
                                    font: Styles.Style.tagFont
                                    color: Styles.Style.secondaryTextColor
                                }
                                
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        systemExpanded = !systemExpanded
                                    }
                                }
                            }
                            
                            // 内容区域
                            Item {
                                id: systemContent
                                anchors.top: systemHeader.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.topMargin: Styles.Style.padding
                                anchors.leftMargin: Styles.Style.padding
                                anchors.rightMargin: Styles.Style.padding
                                height: systemExpanded ? implicitHeight : 0
                                implicitHeight: systemList.implicitHeight
                                clip: true
                                visible: systemExpanded
                                
                                ListView {
                                    id: systemList
                                    width: parent.width
                                    height: parent.height
                                    implicitHeight: contentHeight
                                    model: MenuFileModel {
                                        id: systemFileModel
                                        showSystemOnly: true
                                    }
                                    delegate: fileDelegate
                                    clip: true
                                    
                                    // 键盘事件处理
                                    Keys.onPressed: function(event) {
                                        console.log("systemList: Key pressed:", event.key)
                                        if (event.key === Qt.Key_F5) {
                                            console.log("F5 pressed, refreshing file lists")
                                            userFileModel.refresh()
                                            systemFileModel.refresh()
                                            event.accepted = true
                                        } else if (event.key === Qt.Key_Delete) {
                                            console.log("Delete key pressed in system list, but system files cannot be deleted")
                                            // 系统文件列表不允许删除
                                            event.accepted = true
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            onWidthChanged: {
                filePanelWidth = width
            }
        }
        
        // 中间面板 - 菜单编辑区
        Rectangle {
            id: menuEditor
            SplitView.minimumWidth: 400
            SplitView.preferredWidth: menuEditorWidth
            color: Styles.Style.backgroundColor
            
            Column {
                anchors.fill: parent
                anchors.margins: Styles.Style.padding
                spacing: 0
                
                // 工具栏
                Rectangle {
                    width: parent.width
                    height: Styles.Style.toolbarHeight
                    color: Styles.Style.backgroundColor
                    
                    Text {
                        anchors.left: parent.left
                        text: qsTr("Menu Structure Editor")
                        font: Styles.Style.h1Font
                        color: Styles.Style.textColor
                    }
                }
                
                // 菜单树视图
                Rectangle {
                    width: parent.width
                    height: parent.height - Styles.Style.toolbarHeight
                    color: Styles.Style.backgroundColor
                    
                    // 空配置文件提示
                    Text {
                        anchors.centerIn: parent
                        text: currentMenuModel === null ? qsTr("Please select a configuration file") : 
                               currentMenuModel.rowCount() === 0 ? qsTr("Right-click to add menu items") : ""
                            font: Styles.Style.bodyFont
                        color: Styles.Style.secondaryTextColor
                        visible: currentMenuModel === null || (currentMenuModel !== null && currentMenuModel.rowCount() === 0)
                        z: 1
                    }
                    
                    // 背景区域右键菜单（用于空配置文件）
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.RightButton
                        propagateComposedEvents: true
                        z: 2
                        
                        onClicked: function(mouse) {
                            if (currentMenuModel !== null && currentMenuModel.rowCount() > 0) {
                                // 如果有菜单项，让事件传递给delegate处理
                                mouse.accepted = false
                            } else if (currentMenuModel !== null) {
                                // 空配置文件，显示添加菜单
                                emptyContextMenu.popup(mouse)
                                mouse.accepted = true
                            }
                        }
                    }
                    
                    ScrollView {
                        id: menuScrollView
                        anchors.fill: parent
                        
                        TreeView {
                            id: menuTreeView
                            width: menuScrollView.width
                            height: menuScrollView.height
                            model: currentMenuModel
                            delegate: treeViewDelegate
                            clip: true
                            alternatingRows: true
                            
                            selectionModel: ItemSelectionModel {
                                id: treeSelection
                            }
                        }
                    }
                    
                    // 空配置文件的右键菜单
                    Menu {
                        id: emptyContextMenu
                        
                        MenuItem {
                            text: qsTr("Add Menu")
                            onTriggered: {
                                console.log("Add menu for empty config")
                                if (currentMenuModel !== null) {
                                    var rootIndex = currentMenuModel.index(0, 0)
                                    currentMenuModel.addChildItem(rootIndex, qsTr("New Menu"))
                                    // 保存到文件
                                    menuManager.saveCurrentModel()
                                }
                            }
                        }
                    }
                }
            }
            
            onWidthChanged: {
                menuEditorWidth = width
            }
        }
        
        // 右侧面板 - 属性编辑区
        Rectangle {
            id: propertyPanel
            SplitView.minimumWidth: 300
            SplitView.preferredWidth: propertyPanelWidth
            color: Styles.Style.backgroundColor
            
            ScrollView {
                anchors.fill: parent
                anchors.margins: Styles.Style.padding
                
                Item {
                    width: parent.width
                    height: childrenRect.height
                    
                    Column {
                        width: parent.width
                        spacing: Styles.Style.spacing * 2
                        
                        Text {
                            anchors.left: parent.left
                            text: currentItem ? qsTr("Property Editor: ") + (currentItem.nameLocal || currentItem.name || "") : qsTr("Property Editor")
                            font: Styles.Style.h1Font
                            color: Styles.Style.textColor
                        }
                    
                    // 根节点属性
                    Column {
                        visible: currentItem !== null && currentItem.level === 0
                        width: parent.width
                        spacing: Styles.Style.spacing
                        
                        // Comment
                        Column {
                            width: parent.width
                            spacing: 5
                            
                            Text {
                                text: qsTr("Description")
                                font: Styles.Style.h3Font
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            TextField {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                font: Styles.Style.bodyFont
                                text: currentItem ? currentItem.comment || "" : ""
                                
                                background: Rectangle {
                                    color: Styles.Style.backgroundColor
                                    border.color: Styles.Style.borderColor
                                    border.width: 1
                                    radius: Styles.Style.borderRadius
                                }
                            }
                        }
                        
                        // Comment[zh_CN]
                        Column {
                            width: parent.width
                            spacing: 5
                            
                            Text {
                                text: qsTr("Description (Chinese)")
                                font: Styles.Style.h3Font
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            TextField {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                font: Styles.Style.bodyFont
                                text: currentItem ? currentItem.commentLocal || "" : ""
                                
                                background: Rectangle {
                                    color: Styles.Style.backgroundColor
                                    border.color: Styles.Style.borderColor
                                    border.width: 1
                                    radius: Styles.Style.borderRadius
                                }
                            }
                        }
                        
                        // Version
                        Column {
                            width: parent.width
                            spacing: 5
                            
                            Text {
                                text: qsTr("Version")
                                font: Styles.Style.h3Font
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            TextField {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                font: Styles.Style.bodyFont
                                text: currentItem ? currentItem.version || "" : ""
                                
                                background: Rectangle {
                                    color: Styles.Style.backgroundColor
                                    border.color: Styles.Style.borderColor
                                    border.width: 1
                                    radius: Styles.Style.borderRadius
                                }
                            }
                        }
                    }
                    
                    // 菜单项属性
                    Column {
                        visible: currentItem !== null && currentItem.level > 0
                        width: parent.width
                        spacing: Styles.Style.spacing
                        
                        // Name
                        Column {
                            width: parent.width
                            spacing: 5
                            
                            Text {
                                text: qsTr("Menu Name")
                                font: Styles.Style.h3Font
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            TextField {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                font: Styles.Style.bodyFont
                                text: currentItem ? currentItem.name || "" : ""
                                
                                background: Rectangle {
                                    color: Styles.Style.backgroundColor
                                    border.color: Styles.Style.borderColor
                                    border.width: 1
                                    radius: Styles.Style.borderRadius
                                }
                            }
                        }
                        
                        // Name[zh_CN]
                        Column {
                            width: parent.width
                            spacing: 5
                            
                            Text {
                                text: qsTr("Menu Name (Chinese)")
                                font: Styles.Style.h3Font
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            TextField {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                font: Styles.Style.bodyFont
                                text: currentItem ? currentItem.nameLocal || "" : ""
                                
                                background: Rectangle {
                                    color: Styles.Style.backgroundColor
                                    border.color: Styles.Style.borderColor
                                    border.width: 1
                                    radius: Styles.Style.borderRadius
                                }
                            }
                        }
                        
                        // X-DFM-MenuTypes (checkbox)
                        Column {
                            width: parent.width
                            spacing: 5
                            
                            Text {
                                text: qsTr("Menu Type")
                                font: Styles.Style.h3Font
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            GridLayout {
                                width: parent.width
                                columns: 2
                                rowSpacing: 5
                                columnSpacing: 10
                                
                                CheckBox {
                                    text: "SingleFile"
                                    checked: currentItem && currentItem.menuTypes ? currentItem.menuTypes.indexOf("SingleFile") >= 0 : false
                                }
                                CheckBox {
                                    text: "MultiFiles"
                                    checked: currentItem && currentItem.menuTypes ? currentItem.menuTypes.indexOf("MultiFiles") >= 0 : false
                                }
                                CheckBox {
                                    text: "Filemanager"
                                    checked: currentItem && currentItem.menuTypes ? currentItem.menuTypes.indexOf("Filemanager") >= 0 : false
                                }
                                CheckBox {
                                    text: "SingleDir"
                                    checked: currentItem && currentItem.menuTypes ? currentItem.menuTypes.indexOf("SingleDir") >= 0 : false
                                }
                                CheckBox {
                                    text: "BlankSpace"
                                    checked: currentItem && currentItem.menuTypes ? currentItem.menuTypes.indexOf("BlankSpace") >= 0 : false
                                }
                            }
                        }
                        
                        // X-DFM-SupportSuffix (多行输入框+选择按钮)
                        Column {
                            width: parent.width
                            spacing: 5
                            
                            Text {
                                text: qsTr("Supported Suffixes")
                                font: Styles.Style.h3Font
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            Row {
                                width: parent.width
                                height: Styles.Style.itemHeight * 3
                                spacing: Styles.Style.spacing
                                
                                ScrollView {
                                    width: parent.width - selectButton.width - parent.spacing
                                    height: parent.height
                                    clip: true
                                    
                                    TextArea {
                                        id: suffixTextArea
                                        width: parent.width
                                        height: parent.height
                                        wrapMode: TextArea.Wrap
                                        font: Styles.Style.bodyFont
                                        placeholderText: qsTr("输入支持的后缀，用冒号分隔，如：mp4:avi:mkv")
                                        text: {
                                            if (currentItem && currentItem.supportSuffix) {
                                                return currentItem.supportSuffix.join(":")
                                            }
                                            return ""
                                        }
                                        
                                        background: Rectangle {
                                            color: Styles.Style.backgroundColor
                                            border.color: Styles.Style.borderColor
                                            border.width: 1
                                            radius: Styles.Style.borderRadius
                                        }
                                        
                                        onTextChanged: {
                                            // 更新当前项的后缀列表
                                            if (currentItem) {
                                                var suffixes = text.split(":").filter(function(s) { return s.trim() !== "" })
                                                currentItem.supportSuffix = suffixes
                                            }
                                        }
                                    }
                                }
                                
                                Components.DButton {
                                    id: selectButton
                                    width: 80
                                    // height: parent.height
                                    text: qsTr("选择")
                                     
                                    onClicked: {
                                        fileTypeSelectorDialog.open()
                                    }
                                }
                            }
                        }
                        
                        // PosNum
                        Column {
                            width: parent.width
                            spacing: 5
                            
                            Text {
                                text: qsTr("Position Number")
                                font: Styles.Style.h3Font
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            SpinBox {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                from: 1
                                to: 100
                                value: currentItem ? currentItem.positionNumber || 1 : 1
                                font: Styles.Style.bodyFont
                            }
                        }
                        
                        // Exec (仅在有 Exec 命令时显示)
                        Column {
                            visible: currentItem && currentItem.execCommand && currentItem.execCommand.length > 0
                            width: parent.width
                            spacing: 5
                            
                            Text {
                                text: qsTr("Executable Command")
                                font: Styles.Style.h3Font
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            TextField {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                font: Styles.Style.bodyFont
                                text: currentItem ? currentItem.execCommand || "" : ""
                                
                                background: Rectangle {
                                    color: Styles.Style.backgroundColor
                                    border.color: Styles.Style.borderColor
                                    border.width: 1
                                    radius: Styles.Style.borderRadius
                                }
                            }
                        }
                    }
                    }
                }
                
                // 未选中任何项时的提示
                Text {
                    visible: currentItem === null
                    anchors.centerIn: parent
                    text: qsTr("Please select a menu item")
                    font: Styles.Style.bodyFont
                    color: Styles.Style.secondaryTextColor
                }
            }
            
            onWidthChanged: {
                propertyPanelWidth = width
            }
        }
    }
    
    // 文件列表委托
    Component {
        id: fileDelegate
        
        Rectangle {
            width: ListView.view ? ListView.view.width : 100
            height: Styles.Style.itemHeight
            color: {
                if (model.filePath === selectedFilePath) {
                    return Styles.Style.selectColor
                } else if (mouseArea.containsMouse) {
                    return Styles.Style.hoverColor
                } else {
                    return "transparent"
                }
            }
            
            // 文件名显示或编辑框
            Loader {
                id: fileNameLoader
                anchors.left: parent.left
                anchors.leftMargin: Styles.Style.padding
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: Styles.Style.padding + (model.isSystem ? 50 : 0)
                
                // 判断是否应该显示编辑框
                property bool shouldShowEdit: {
                    // 新建文件的占位符项（路径为空）
                    if (isNewFile && model.filePath === "" && editingModelRef) {
                        return true
                    }
                    // 重命名现有文件
                    if (!isNewFile && editingFilePath === model.filePath && !model.isSystem) {
                        return true
                    }
                    return false
                }
                
                sourceComponent: shouldShowEdit ? editComponent : textComponent
            }
            
            Component {
                id: textComponent
                Text {
                    text: model.fileName || ""
                    font.pixelSize: Styles.Style.bodyFont.pixelSize
                    font.family: Styles.Style.bodyFont.family
                    font.bold: model.filePath === selectedFilePath
                    color: Styles.Style.textColor
                    elide: Text.ElideRight
                }
            }
            
            Component {
                id: editComponent
                TextInput {
                    id: textInput
                    text: {
                        if (isNewFile) {
                            return ""
                        } else {
                            // 移除 .conf 扩展名用于编辑
                            var fileName = model.fileName || ""
                            return fileName.replace(/\.conf$/, '')
                        }
                    }
                    font.pixelSize: Styles.Style.bodyFont.pixelSize
                    font.family: Styles.Style.bodyFont.family
                    color: Styles.Style.textColor
                    selectByMouse: true
                    selectionColor: Styles.Style.primaryColor
                    
                    onAccepted: {
                        finishEditing(text)
                    }
                    
                    onEditingFinished: {
                        // 修改逻辑：如果不是新建文件，并且（editingFilePath 匹配或为空），则保存
                        // 这样可以处理点击其他文件时 editingFilePath 被清空的情况
                        if (!isNewFile && (editingFilePath === model.filePath || editingFilePath === "")) {
                            // 只有当文本不为空时才保存
                            if (text.trim() !== "") {
                                finishEditing(text)
                            }
                        }
                    }
                    
                    Keys.onPressed: function(event) {
                        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                            event.accepted = true
                            finishEditing(text)
                        } else if (event.key === Qt.Key_Escape) {
                            event.accepted = true
                            cancelEditing()
                        }
                    }
                    
                    onFocusChanged: {
                        if (!focus && (fileNameLoader.shouldShowEdit)) {
                            console.log("TextInput focus lost, finishing edit")
                            finishEditing(text)
                        }
                    }
                    
                    Component.onCompleted: {
                        forceActiveFocus()
                        selectAll()
                    }
                }
            }
            
            Text {
                anchors.right: parent.right
                anchors.rightMargin: Styles.Style.padding
                anchors.verticalCenter: parent.verticalCenter
                text: model.isSystem ? qsTr("System") : ""
                font: Styles.Style.tagFont
                color: Styles.Style.systemTagColor
                visible: model.isSystem
            }
            
            MouseArea {
                id: mouseArea
                // 根据文件路径判断是用户文件还是系统文件
                property var currentModel: model.filePath && model.filePath.indexOf("/.local/share/") !== -1 ? userFileModel : 
                                          model.filePath && model.filePath.indexOf("/usr/share/") !== -1 ? systemFileModel : null
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                // 禁用点击，让Loader处理
                enabled: !fileNameLoader.shouldShowEdit
                
                onDoubleClicked: function(mouse) {
                    if (mouse.button === Qt.LeftButton && model.filePath !== "" && !model.isSystem) {
                        console.log("Double-clicked on file:", model.filePath)
                        console.log("currentModel:", currentModel, "typeof:", typeof currentModel)
                        editingFilePath = model.filePath
                        isNewFile = false
                        editingModelRef = currentModel
                        console.log("Set editingModelRef to:", editingModelRef)
                    }
                }
                
                onClicked: function(mouse) {
                    if (mouse.button === Qt.LeftButton) {
                        // 如果有其他文件正在编辑，先结束编辑
                        if (editingFilePath !== "" && editingFilePath !== model.filePath) {
                            console.log("Another file is being edited, ending edit first")
                            // 清空编辑状态，这会导致 Loader 切换回 textComponent
                            // TextInput 的 onEditingFinished 会被触发，但由于 editingFilePath 已被清空
                            // 我们需要修改 onEditingFinished 的逻辑来处理这种情况
                            editingFilePath = ""
                            isNewFile = false
                            editingModelRef = null
                        }
                        selectedFilePath = model.filePath || ""
                        menuManager.setCurrentConfig(model.filePath || "")
                        contextMenu.filePath = model.filePath || ""
                    } else if (mouse.button === Qt.RightButton) {
                        // 如果有其他文件正在编辑，先结束编辑
                        if (editingFilePath !== "" && editingFilePath !== model.filePath) {
                            console.log("Another file is being edited, ending edit first for right-click")
                            editingFilePath = ""
                            isNewFile = false
                            editingModelRef = null
                        }
                        contextMenu.filePath = model.filePath || ""
                        contextMenu.modelRef = currentModel
                        console.log("Right-click: setting contextMenu.modelRef to", currentModel, "typeof:", typeof currentModel)
                        contextMenu.popup()
                    }
                }
            }
            
            // 完成编辑的函数
            function finishEditing(newName) {
                var trimmedName = newName.trim()
                console.log("finishEditing called with:", trimmedName, "isNewFile:", isNewFile, "editingFilePath:", editingFilePath, "model.filePath:", model.filePath)
                
                if (trimmedName !== "") {
                    console.log("Finishing edit with name:", trimmedName)
                    var currentModel = ListView.view ? ListView.view.model : null
                    console.log("currentModel:", currentModel, "editingModelRef:", editingModelRef, "typeof editingModelRef:", typeof editingModelRef)
                    
                    if (isNewFile && model.filePath === "" && editingModelRef && editingModelRef.createFile) {
                        // 创建新文件
                        console.log("Creating new file:", trimmedName)
                        editingModelRef.createFile(trimmedName)
                    } else if (!isNewFile && editingFilePath !== "" && editingModelRef && editingModelRef.renameFile) {
                        // 重命名文件
                        console.log("Renaming file from", editingFilePath, "to", trimmedName)
                        editingModelRef.renameFile(editingFilePath, trimmedName)
                    } else {
                        console.log("Condition not met - isNewFile:", isNewFile, "editingFilePath:", editingFilePath, "model.filePath:", model.filePath, "editingModelRef:", editingModelRef, "has createFile:", editingModelRef && editingModelRef.createFile, "has renameFile:", editingModelRef && editingModelRef.renameFile)
                    }
                }
                cancelEditing()
            }
            
            // 取消编辑的函数
            function cancelEditing() {
                console.log("cancelEditing called - isNewFile:", isNewFile, "model.filePath:", model.filePath)
                if (isNewFile && model.filePath === "" && editingModelRef && editingModelRef.cancelNewFile) {
                    // 取消新建文件，移除占位符
                    console.log("Calling cancelNewFile")
                    editingModelRef.cancelNewFile()
                }
                editingFilePath = ""
                isNewFile = false
                editingModelRef = null
            }
        }
    }
    
    // 折叠状态
    property bool userExpanded: true
    property bool systemExpanded: false
    
    // 当前选中的文件路径
    property string selectedFilePath: ""

    // 当前菜单树模型
    property var currentMenuModel: null
    
    // 编辑状态管理
    property string editingFilePath: ""  // 当前正在编辑的文件路径
    property bool isNewFile: false       // 是否是新建文件
    property var editingModelRef: null   // 正在编辑的模型引用
    
    // 右键上下文菜单（移到ApplicationWindow级别以访问所有模型）
    Menu {
        id: contextMenu
        property string filePath: ""
        property var modelRef: null
        
        MenuItem {
            text: qsTr("Refresh List")
            onTriggered: {
                console.log("Refreshing file lists")
                userFileModel.refresh()
                systemFileModel.refresh()
            }
        }
        
        MenuSeparator {}
        
        MenuItem {
            text: qsTr("New File")
            onTriggered: {
                console.log("Creating new file inline")
                var currentModel = contextMenu.modelRef
                if (currentModel && currentModel.startNewFile) {
                    currentModel.startNewFile()
                    // 设置编辑状态
                    editingFilePath = ""
                    isNewFile = true
                    editingModelRef = currentModel
                }
            }
        }
        
        MenuItem {
            text: qsTr("Rename")
            enabled: contextMenu.filePath !== ""
            onTriggered: {
                console.log("Rename triggered: contextMenu.modelRef =", contextMenu.modelRef, "typeof:", typeof contextMenu.modelRef)
                console.log("Starting inline edit for:", contextMenu.filePath)
                editingFilePath = contextMenu.filePath
                isNewFile = false
                editingModelRef = contextMenu.modelRef
                console.log("Set editingModelRef to:", editingModelRef)
            }
        }
        
        MenuItem {
            text: qsTr("Delete")
            enabled: contextMenu.filePath !== ""
            onTriggered: {
                console.log("Deleting file:", contextMenu.filePath)
                if (contextMenu.modelRef === userFileModel) {
                    userFileModel.deleteFile(contextMenu.filePath)
                } else if (contextMenu.modelRef === systemFileModel) {
                    systemFileModel.deleteFile(contextMenu.filePath)
                }
            }
        }
        
        MenuSeparator {}
        
        MenuItem {
            text: qsTr("Open Containing Folder")
            enabled: contextMenu.filePath !== ""
            onTriggered: {
                console.log("Opening containing folder for:", contextMenu.filePath)
                if (contextMenu.modelRef === userFileModel) {
                    console.log("Calling userFileModel.openContainingFolder")
                    userFileModel.openContainingFolder(contextMenu.filePath)
                } else if (contextMenu.modelRef === systemFileModel) {
                    console.log("Calling systemFileModel.openContainingFolder")
                    systemFileModel.openContainingFolder(contextMenu.filePath)
                } else {
                    console.log("Unknown model, trying both")
                    userFileModel.openContainingFolder(contextMenu.filePath)
                }
            }
        }
        
        MenuItem {
            text: qsTr("Open File")
            enabled: contextMenu.filePath !== ""
            onTriggered: {
                console.log("Opening file:", contextMenu.filePath)
                if (contextMenu.modelRef === userFileModel) {
                    console.log("Calling userFileModel.openFile")
                    userFileModel.openFile(contextMenu.filePath)
                } else if (contextMenu.modelRef === systemFileModel) {
                    console.log("Calling systemFileModel.openFile")
                    systemFileModel.openFile(contextMenu.filePath)
                } else {
                    console.log("Unknown model, trying both")
                    userFileModel.openFile(contextMenu.filePath)
                }
            }
        }
    }
    
    // 当前选中的菜单项
    property var currentItem: null
    // 当前正在编辑的 delegate
    property var editingDelegate: null
    
    // 保存列宽
    property real filePanelWidth: 350
    property real menuEditorWidth: 630
    property real propertyPanelWidth: 420
    
    // 连接 MenuManager 信号
    Connections {
        target: menuManager
        function onConfigLoaded(configFile) {
            console.log("Config loaded:", configFile)
            selectedFilePath = configFile
            currentMenuModel = menuManager.getMenuModel(configFile)
            if (currentMenuModel) {
                console.log("Menu model loaded, row count:", currentMenuModel.rowCount())
                
                // 导出 JSON 用于调试
                var jsonString = menuManager.exportToJson(configFile)
                console.log("=== Exported JSON ===")
                console.log(jsonString)
                console.log("=== End of JSON ===")
            } else {
                console.log("Failed to load menu model")
            }
        }
    }
    
    // 文件类型选择弹窗
    Components.DFileTypeSelectorDialog {
        id: fileTypeSelectorDialog
        parent: root
        
        allFileTypes: fileTypeManager.allFileTypes
        categories: fileTypeManager.categories
        
        onOpened: {
            // 初始化已选择的后缀列表
            if (currentItem && currentItem.supportSuffix) {
                selectedSuffixes = currentItem.supportSuffix.slice()  // 复制数组
            } else {
                selectedSuffixes = []
            }
            console.log("Dialog opened, current suffixes:", selectedSuffixes)
        }
        
        onAccepted: {
            // 确认选择，更新后缀列表
            if (currentItem) {
                currentItem.supportSuffix = selectedSuffixes.slice()  // 复制数组
                // 更新文本框显示
                suffixTextArea.text = selectedSuffixes.join(":")
                console.log("Suffixes updated:", selectedSuffixes.join(":"))
            }
        }
        
        onSelectionChanged: function(suffixes) {
            console.log("Selection changed:", suffixes.join(":"))
        }
    }
}

