import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DFMMenu 1.0
import "qrc:/qml/styles" as Styles

ApplicationWindow {
    id: root
    visible: true
    width: 1400
    height: 900
    minimumWidth: 1000
    minimumHeight: 600
    title: qsTr("DFM 右键菜单管理器")
    
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
                height: parent.height
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
                font.pixelSize: 12
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
                font.pixelSize: 14
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
        // WindowManager.restoreState(root)
    }
    
    Component.onDestruction: {
        WindowManager.saveState(root)
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
                    placeholderText: qsTr("搜索配置文件...")
                    font.pixelSize: 14
                    
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
                                    text: qsTr("用户配置")
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: Styles.Style.textColor
                                }
                                
                                Text {
                                    anchors.right: parent.right
                                    anchors.rightMargin: Styles.Style.padding
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: userExpanded ? "▼" : "▶"
                                    font.pixelSize: 12
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
                                anchors.margins: Styles.Style.padding
                                height: userExpanded ? 400 : 0
                                clip: true
                                
                                ListView {
                                    id: userList
                                    anchors.fill: parent
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
                                    text: qsTr("系统配置")
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: Styles.Style.textColor
                                }
                                
                                Text {
                                    anchors.right: parent.right
                                    anchors.rightMargin: Styles.Style.padding
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: systemExpanded ? "▼" : "▶"
                                    font.pixelSize: 12
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
                                anchors.margins: Styles.Style.padding
                                height: systemExpanded ? 400 : 0
                                clip: true
                                
                                ListView {
                                    id: systemList
                                    anchors.fill: parent
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
                spacing: 0
                
                // 工具栏
                Rectangle {
                    width: parent.width
                    height: Styles.Style.toolbarHeight
                    color: Styles.Style.backgroundColor
                    
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("菜单结构编辑")
                        font.pixelSize: 16
                        font.bold: true
                        color: Styles.Style.textColor
                    }
                }
                
                // 菜单树视图
                Rectangle {
                    width: parent.width
                    height: parent.height - Styles.Style.toolbarHeight
                    color: Styles.Style.backgroundColor
                    
                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: Styles.Style.padding
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
                    
                    Text {
                        anchors.centerIn: parent
                        text: currentMenuModel === null ? qsTr("请选择一个配置文件") : ""
                        font.pixelSize: 14
                        color: Styles.Style.secondaryTextColor
                        visible: currentMenuModel === null
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
                            text: currentItem ? qsTr("属性编辑: ") + (currentItem.nameLocal || currentItem.name || "") : qsTr("属性编辑")
                            font.pixelSize: 16
                            font.bold: true
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
                                text: qsTr("描述")
                                font.pixelSize: 12
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            TextField {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                font.pixelSize: 14
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
                                text: qsTr("描述(中文)")
                                font.pixelSize: 12
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            TextField {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                font.pixelSize: 14
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
                                text: qsTr("版本号")
                                font.pixelSize: 12
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            TextField {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                font.pixelSize: 14
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
                                text: qsTr("菜单名称")
                                font.pixelSize: 12
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            TextField {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                font.pixelSize: 14
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
                                text: qsTr("菜单名称(中文)")
                                font.pixelSize: 12
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            TextField {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                font.pixelSize: 14
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
                                text: qsTr("菜单类型")
                                font.pixelSize: 12
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
                        
                        // X-DFM-SupportSuffix (简化版，只显示几个常用的)
                        Column {
                            width: parent.width
                            spacing: 5
                            
                            Text {
                                text: qsTr("支持的后缀")
                                font.pixelSize: 12
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            GridLayout {
                                width: parent.width
                                columns: 3
                                rowSpacing: 5
                                columnSpacing: 10
                                
                                CheckBox {
                                    text: "视频"
                                    checked: currentItem && currentItem.supportSuffix ? currentItem.supportSuffix.some(suffix => suffix.startsWith("mp4") || suffix.startsWith("avi") || suffix.startsWith("mkv")) : false
                                }
                                CheckBox {
                                    text: "音频"
                                    checked: currentItem && currentItem.supportSuffix ? currentItem.supportSuffix.some(suffix => suffix.startsWith("mp3") || suffix.startsWith("wav") || suffix.startsWith("flac")) : false
                                }
                                CheckBox {
                                    text: "图片"
                                    checked: currentItem && currentItem.supportSuffix ? currentItem.supportSuffix.some(suffix => suffix.startsWith("jpg") || suffix.startsWith("png") || suffix.startsWith("gif")) : false
                                }
                                CheckBox {
                                    text: "文档"
                                    checked: currentItem && currentItem.supportSuffix ? currentItem.supportSuffix.some(suffix => suffix.startsWith("pdf") || suffix.startsWith("doc") || suffix.startsWith("txt")) : false
                                }
                            }
                        }
                        
                        // PosNum
                        Column {
                            width: parent.width
                            spacing: 5
                            
                            Text {
                                text: qsTr("位置编号")
                                font.pixelSize: 12
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            SpinBox {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                from: 1
                                to: 100
                                value: currentItem ? currentItem.positionNumber || 1 : 1
                                font.pixelSize: 14
                            }
                        }
                        
                        // Exec (仅在有 Exec 命令时显示)
                        Column {
                            visible: currentItem && currentItem.execCommand && currentItem.execCommand.length > 0
                            width: parent.width
                            spacing: 5
                            
                            Text {
                                text: qsTr("可执行命令")
                                font.pixelSize: 12
                                color: Styles.Style.secondaryTextColor
                            }
                            
                            TextField {
                                width: parent.width
                                height: Styles.Style.itemHeight
                                font.pixelSize: 14
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
                
                // 未选中任何项时的提示（在ScrollView底部）
                Text {
                    visible: currentItem === null
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: Styles.Style.padding * 2
                    text: qsTr("请选择一个菜单项")
                    font.pixelSize: 14
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
                font.pixelSize: 14
                color: Styles.Style.textColor
                font.bold: model.filePath === selectedFilePath
            }
            
            Text {
                anchors.right: parent.right
                anchors.rightMargin: Styles.Style.padding
                anchors.verticalCenter: parent.verticalCenter
                text: model.isSystem ? qsTr("系统") : ""
                font.pixelSize: 12
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
}
