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
            
            required property TreeView treeView
            required property bool isTreeNode
            required property bool expanded
            required property int hasChildren
            required property int depth
            
            color: "transparent"
            
            border.color: "transparent"
            border.width: 1
            radius: 4
            
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
                        treeView.toggleExpanded(row)
                        console.log("Toggle expand:", model.name, "expanded:", !delegateItem.expanded, "hasChildren:", delegateItem.hasChildren)
                    }
                }
            }
            
            // 文本内容
            Text {
                id: label
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
                        // 更新当前选中的菜单项
                        currentItem = model
                    }
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
                    
                    ScrollView {
                        id: menuScrollView
                        anchors.fill: parent
                        visible: currentMenuModel !== null
                        
                        TreeView {
                            id: menuTreeView
                            anchors.fill: parent
                            model: currentMenuModel
                            delegate: treeViewDelegate
                            clip: true
                            alternatingRows: true
                            
                            selectionModel: ItemSelectionModel {
                                id: treeSelection
                            }
                        }
                    }
                }
            }

            Text {
                anchors.centerIn: parent
                text: currentMenuModel === null ? qsTr("Please select a configuration file") : ""
                font: Styles.Style.bodyFont
                color: Styles.Style.secondaryTextColor
                visible: currentMenuModel === null
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
            
            Text {
                anchors.left: parent.left
                anchors.leftMargin: Styles.Style.padding
                anchors.verticalCenter: parent.verticalCenter
                text: model.fileName || ""
                font.pixelSize: Styles.Style.bodyFont.pixelSize
                font.family: Styles.Style.bodyFont.family
                font.bold: model.filePath === selectedFilePath
                color: Styles.Style.textColor
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
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    selectedFilePath = model.filePath || ""
                    menuManager.setCurrentConfig(model.filePath || "")
                }
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
    
    // 当前选中的菜单项
    property var currentItem: null
    
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

