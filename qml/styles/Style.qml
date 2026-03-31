// SPDX-FileCopyrightText: 2026 zhanghongyuan <zhanghongyuan@uniontech.com>
// SPDX-License-Identifier: GPL-3.0-or-later
pragma Singleton
import QtQuick

QtObject {
    // 颜色
    readonly property color primaryColor: "#005FB8"
    readonly property color backgroundColor: "#FFFFFF"
    readonly property color borderColor: "#E5E5E5"
    readonly property color textColor: "#000000"
    readonly property color secondaryTextColor: "#888888"
    readonly property color selectColor: "#E8F3FF"
    readonly property color hoverColor: "#F5F5F5"
    readonly property color systemTagColor: "#FF6A00"
    readonly property color errorColor: "#FF5722"
    readonly property color successColor: "#00C853"
    
    // 字体
    // 一级标题（如：菜单结构编辑、属性编辑）
    readonly property font h1Font: Qt.font({
        family: "Noto Sans CJK SC",
        pixelSize: 18,
        bold: true
    })
    
    // 二级标题（如：用户配置、系统配置）
    readonly property font h2Font: Qt.font({
        family: "Noto Sans CJK SC",
        pixelSize: 15,
        bold: true
    })
    
    // 三级标题（如：描述、菜单名称等标签）
    readonly property font h3Font: Qt.font({
        family: "Noto Sans CJK SC",
        pixelSize: 13,
        bold: false
    })
    
    // 正文内容
    readonly property font bodyFont: Qt.font({
        family: "Noto Sans CJK SC",
        pixelSize: 14
    })
    
    // 标签/小字
    readonly property font tagFont: Qt.font({
        family: "Noto Sans CJK SC",
        pixelSize: 12
    })
    
    // 小字（辅助信息）
    readonly property font smallFont: Qt.font({
        family: "Noto Sans CJK SC",
        pixelSize: 11
    })
    
    // 尺寸
    readonly property int inset: 2
    readonly property int spacing: 6
    readonly property int padding: 8
    readonly property int borderRadius: 4
    readonly property int itemHeight: 40
    readonly property int toolbarHeight: 50
    readonly property int borderWidth: 1
    readonly property int activeBorderWidth: 2
    readonly property int iconSize: 16
    readonly property int smallBorderRadius: 3
    
    // 动画
    readonly property int animationDuration: 150
}
