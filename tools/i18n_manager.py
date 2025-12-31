#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Media Analyzer 统一国际化管理工具
集成所有翻译相关功能到一个脚本中
"""

import os
import sys
import subprocess
import xml.etree.ElementTree as ET
from pathlib import Path

class I18nManager:
    def __init__(self, project_root: str = None):
        self.project_root = Path(project_root) if project_root else Path(__file__).parent.parent
        self.translations_dir = self.project_root / "translations"
        self.pro_file = self.project_root / "media-analyzer.pro"
        
        # 支持的语言
        self.languages = {
            "zh_CN": "简体中文",
            "en_US": "English", 
            "ja_JP": "日本語",
            "ko_KR": "한국어",
            "fr_FR": "Français",
            "de_DE": "Deutsch",
            "es_ES": "Español",
            "ru_RU": "Русский",
            "ar_SA": "العربية",
            "pt_BR": "Português (Brasil)"
        }
        
        # 中文翻译映射
        self.cn_translations = {
            # 基本界面元素
            "Form": "表单", "File": "文件", "Edit": "编辑", "View": "视图",
            "Help": "帮助", "Config": "配置", "Tools": "工具", "Play": "播放",
            "Stop": "停止", "Pause": "暂停", "Open": "打开", "Close": "关闭",
            "Save": "保存", "Export": "导出", "Import": "导入", "Delete": "删除",
            "Remove": "移除", "Add": "添加", "Search": "搜索", "Clear": "清除",
            "Cancel": "取消", "OK": "确定", "Apply": "应用", "Reset": "重置",
            "Default": "默认", "Advanced": "高级", "Basic": "基本", "General": "常规",
            "Settings": "设置", "Options": "选项", "Properties": "属性",
            "Information": "信息", "Details": "详情", "Summary": "摘要",
            "Status": "状态", "Progress": "进度", "Error": "错误",
            "Warning": "警告", "Info": "信息", "Debug": "调试",
            
            # 媒体相关
            "Media": "媒体", "Media Info": "媒体信息", "Media Properties": "媒体属性",
            "Media Player": "媒体播放器", "Streams": "流", "Format": "格式",
            "Formats": "格式", "Video": "视频", "Audio": "音频", "Frames": "帧",
            "Packets": "包", "Chapters": "章节", "Programs": "节目",
            "Duration": "时长", "Bitrate": "比特率", "Resolution": "分辨率",
            "Codec": "编解码器", "Codecs": "编解码器", "Encoders": "编码器",
            "Decoders": "解码器", "Muxers": "复用器", "Demuxers": "解复用器",
            "Filters": "滤镜", "Protocols": "协议", "Devices": "设备",
            "Sources": "源", "Sinks": "输出",
            
            # 操作相关
            "Select": "选择", "Select All": "全选", "Select None": "全不选",
            "Copy": "复制", "Copy File Path": "复制文件路径",
            "Open File Location": "打开文件位置", "Play": "播放",
            "Remove Record": "移除记录", "Delete File": "删除文件",
            
            # 搜索相关
            "Search Range": "搜索范围", "Match Control": "匹配控制",
            "Case Sensitive": "区分大小写", "Match Whole words": "全词匹配",
            "Use Regular Expression": "使用正则表达式", "Detail Search": "详细搜索",
            "Search text is empty": "搜索文本为空", "No content to search": "没有内容可搜索",
            "Found %1 of %2 items": "找到 %2 项中的第 %1 项", "No items found": "未找到项目",
            
            # 文件操作
            "Open File": "打开文件", "Open Files": "打开文件", "Open Folder": "打开文件夹",
            "Select Files": "选择文件", "All Files (*.*)": "所有文件 (*.*)",
            "All Files (*.*);;Media Files (*.mp4 *.mkv *.webm *.mp3);;Image Files (*.png *.jpg *.bmp)": "所有文件 (*.*);;媒体文件 (*.mp4 *.mkv *.webm *.mp3);;图像文件 (*.png *.jpg *.bmp)",
            "Select Save Directory": "选择保存目录", "Open Save Directory": "打开保存目录",
            "Cannot create directory: %1": "无法创建目录: %1",
            
            # 导出相关
            "Export Files": "导出文件", "Operation": "操作", "Input Save Dir": "输入保存目录",
            "Input Save File Name": "输入保存文件名", "Preview": "预览",
            "Basic Info Fileds": "基本信息字段", "Media Info Fileds": "媒体信息字段",
            "compact(csv)": "紧凑(CSV)", "ini": "INI", "json": "JSON", "xml": "XML",
            "default": "默认", "hierarchical": "层次结构", "flat": "扁平",
            
            # FFmpeg 相关
            "FFmpeg Builder": "FFmpeg 构建器", "FFmepg Select": "FFmpeg 选择",
            "Local FFmpeg:": "本地 FFmpeg:", "Remote FFmpeg:": "远程 FFmpeg:",
            "Select Options": "选择选项", "Build": "构建", "Install": "安装",
            "Configure": "配置", "Clean": "清理", "Complie Options": "编译选项",
            "Complie Outputs": "编译输出", "Select FFmpeg source folder": "选择 FFmpeg 源文件夹",
            
            # 日志相关
            "Logs": "日志", "Clear Logs": "清除日志",
            "Toggle Search Detail (Ctrl+F)": "切换搜索详情 (Ctrl+F)",
            "Toggle View (Ctrl+T)": "切换视图 (Ctrl+T)",
            
            # 窗口和布局
            "MainWindow": "主窗口", "Files": "文件", "Folders": "文件夹", "Player": "播放器",
            "Log": "日志", "Mainwindow": "主窗口", "Window Head": "窗口标题",
            "Column Visibility": "列可见性", "Show All": "显示全部",
            "Hide Vertical Header": "隐藏垂直标题", "Show Vertical Header": "显示垂直标题",
            "Hide Total Count": "隐藏总计", "Show Total Count": "显示总计",
            "Column %1": "列 %1",
            
            # 基本信息
            "Basic Info": "基本信息", "Version": "版本", "Buildconf": "构建配置",
            "Build": "构建", "License": "许可证", "About": "关于",
            "About MediaAnalyzer": "关于 MediaAnalyzer",
            
            # 帮助相关
            "Help Query %1=%2": "帮助查询 %1=%2", "No help information available for %1=%2": "没有 %1=%2 的帮助信息",
            "Failed to load %1 parameters": "加载 %1 参数失败", "Keep Last": "保持上次",
            "Category": "类别", "Category:": "类别:",
            
            # 进度对话框
            "Processing...": "处理中...", "Please wait...": "请稍候...",
            "Elapsed time: 00:00:00": "已用时间: 00:00:00", "Elapsed time: %1": "已用时间: %1",
            
            # 命令执行
            "Starting execution of %1 commands...": "开始执行 %1 个命令...",
            "Execution stopped by user": "用户停止执行", "Executing command %1/%2: %3": "执行命令 %1/%2: %3",
            "Failed to start process": "启动进程失败", "Process timeout": "进程超时",
            "Completed %1/%2 commands": "完成 %1/%2 个命令", "All commands completed successfully": "所有命令成功完成",
            "Some commands failed or were stopped": "部分命令失败或被停止",
            "Output from index %1 %2: %3": "索引 %1 %2 的输出: %3",
            "Error from index %1 %2: %3": "索引 %1 %2 的错误: %3",
            "Started: %1 %2": "开始: %1 %2", "not exists! will use default:": "不存在！将使用默认值:",
            
            # 其他界面元素
            "Name": "名称", "Input": "输入", "Time": "时间", "Previous": "上一个",
            "Next": "下一个", "yyyy-MM-dd HH:mm:ss zzz": "yyyy-MM-dd HH:mm:ss zzz",
            "<< - >>": "<< - >>", "suffix:": "后缀:", "item_sep:": "项分隔符:",
            "escape:": "转义:", "print_section": "打印节", "noprint_wrappers": "不打印包装器",
            "sep_char:": "分隔字符:", "fully_qualified": "完全限定", "xsd_strict": "XSD 严格",
            
            # 媒体分析相关
            "Parse Media: %1": "解析媒体: %1", "Basic info get error": "获取基本信息错误",
            "%1 - Media Properties: %2": "%1 - 媒体属性: %2", "Properties: %1": "属性: %1",
            " %1 a:0": " %1 a:0", " %1 v:0": " %1 v:0",
            
            # 关于对话框
            "MediaAnalyzer\n\nA powerful media file analysis tool that provides:\n- Detailed media information display\n- Multiple format views (JSON, Table)\n- Real-time logging\n- Customizable layout\n- FFmpeg compilation tool\n\nVersion 1.0": 
            "MediaAnalyzer\n\n一个强大的媒体文件分析工具，提供：\n- 详细的媒体信息显示\n- 多种格式视图（JSON、表格）\n- 实时日志记录\n- 可自定义的布局\n- FFmpeg 编译工具\n\n版本 1.0",
            
            # 错误消息
            "No input file.": "没有输入文件。",
            "Error: Please specify path of media file": "错误：请指定媒体文件路径",
            "Error: Please specify frame type, (audio, video, a, v)": "错误：请指定帧类型，(audio, video, a, v)",
            "Error: Please specify frame type, (frame, packet, f, p)": "错误：请指定帧类型，(frame, packet, f, p)",
            "Error: Media file information cannot be obtained. Please check if file path is correct": "错误：无法获取媒体文件信息。请检查文件路径是否正确",
            "Error: Unsupported basic information type. The supported types include: \nversion, buildconf, formats, muxers, demuxers, devices, codecs, decoders, encoders, bsfs, protocols, filters, pixfmts, layouts, samplefmts, colors, license": 
            "错误：不支持的基本信息类型。支持的类型包括：\nversion, buildconf, formats, muxers, demuxers, devices, codecs, decoders, encoders, bsfs, protocols, filters, pixfmts, layouts, samplefmts, colors, license",
            
            # 媒体格式相关
            "Pixel Formats": "像素格式", "VideoRate": "视频帧率", "VideoSize": "视频尺寸",
            "Video Frame": "视频帧", "SampleFmts": "采样格式", "Layouts": "布局",
            "Colors": "颜色", "ProgramVersion": "程序版本", "LibraryVersions": "库版本",
            "Versions": "版本", "Count Frames": "计数帧", "Count Packets": "计数包",
            "Entries": "条目",
            
            # 应用程序目录
            "App Dir": "应用目录", "Setting Dir": "设置目录", "Setting File": "设置文件",
            
            # 字体相关
            "微软雅黑": "微软雅黑",
            
            # JSON 相关
            "Copy Value": "复制值", "Copy Key-Value": "复制键值", "Copy Key": "复制键",
            "Copy All Data": "复制所有数据", "Expand All": "展开全部", "Collapse All": "折叠全部",
            "JSON Search": "JSON 搜索",
            
            # 表格相关
            "Input seach Text": "输入搜索文本", "name": "名称", "NB_COMPONENTS": "组件数量",
            "BITS_PER_PIXEL": "每像素位数", "direct": "直接", "detail": "详情",
            "Open advanced search dialog with more options": "打开具有更多选项的高级搜索对话框",
            
            # 其他
            "Bsfs": "比特流滤镜", "Pixfmts": "像素格式", "Docs Viewer": "文档查看器",
        }
    
    def update_translations(self):
        """更新翻译文件"""
        print("更新翻译文件...")
        
        cmd = [
            'lupdate',
            str(self.pro_file),
            '-ts',
            '-no-obsolete'
        ]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, cwd=self.project_root)
            if result.returncode == 0:
                print("✓ 翻译文件更新成功")
                return True
            else:
                print(f"✗ 翻译文件更新失败: {result.stderr}")
                return False
        except FileNotFoundError:
            print("✗ 找不到 lupdate 命令，请确保 Qt 工具已安装")
            return False
    
    def release_translations(self):
        """生成.qm翻译文件"""
        print("生成翻译文件...")
        
        ts_files = list(self.translations_dir.glob("*.ts"))
        if not ts_files:
            print("✗ 没有找到翻译文件")
            return False
        
        for ts_file in ts_files:
            cmd = ['lrelease', str(ts_file)]
            try:
                result = subprocess.run(cmd, capture_output=True, text=True, cwd=self.project_root)
                if result.returncode == 0:
                    lang_name = self.languages.get(ts_file.stem.split('_')[-1], ts_file.stem)
                    print(f"✓ {lang_name} 翻译文件生成成功")
                else:
                    print(f"✗ {ts_file.name} 翻译文件生成失败: {result.stderr}")
            except FileNotFoundError:
                print("✗ 找不到 lrelease 命令，请确保 Qt 工具已安装")
                return False
        
        return True
    
    def complete_chinese_translation(self):
        """补全中文翻译"""
        print("补全中文翻译...")
        
        ts_file = self.translations_dir / "media-analyzer_zh_CN.ts"
        if not ts_file.exists():
            print(f"✗ 翻译文件不存在: {ts_file}")
            return False
        
        try:
            tree = ET.parse(ts_file)
            root = tree.getroot()
            
            translated_count = 0
            for context in root.findall('context'):
                context_name = context.find('name').text
                print(f"处理上下文: {context_name}")
                
                for message in context.findall('message'):
                    source = message.find('source')
                    if source is not None and source.text:
                        translation = message.find('translation')
                        if translation is not None:
                            # 如果已经有翻译且不是unfinished，跳过
                            if translation.text and translation.get('type') != 'unfinished':
                                continue
                            
                            # 查找翻译
                            translated_text = self.cn_translations.get(source.text)
                            if translated_text:
                                translation.text = translated_text
                                if 'type' in translation.attrib:
                                    del translation.attrib['type']
                                print(f"  翻译: '{source.text}' -> '{translated_text}'")
                                translated_count += 1
            
            # 保存文件
            tree.write(ts_file, encoding='utf-8', xml_declaration=True)
            print(f"✓ 翻译文件已保存: {ts_file}")
            print(f"✓ 共翻译了 {translated_count} 个字符串")
            return True
            
        except Exception as e:
            print(f"✗ 处理翻译文件失败: {e}")
            return False
    
    def list_languages(self):
        """列出支持的语言"""
        print("支持的语言:")
        for code, name in self.languages.items():
            ts_file = self.translations_dir / f"media-analyzer_{code}.ts"
            qm_file = self.translations_dir / f"media-analyzer_{code}.qm"
            ts_status = "✓" if ts_file.exists() else "✗"
            qm_status = "✓" if qm_file.exists() else "✗"
            print(f"  {code}: {name} [{ts_status} TS] [{qm_status} QM]")
    
    def open_linguist(self, lang_code=None):
        """打开Qt Linguist"""
        if lang_code and lang_code in self.languages:
            ts_file = self.translations_dir / f"media-analyzer_{lang_code}.ts"
            if ts_file.exists():
                cmd = ['linguist', str(ts_file)]
                try:
                    subprocess.Popen(cmd, cwd=self.project_root)
                    print(f"✓ 已打开 {self.languages[lang_code]} 翻译文件")
                except FileNotFoundError:
                    print("✗ 找不到 linguist 命令，请确保 Qt 工具已安装")
            else:
                print(f"✗ 翻译文件不存在: {ts_file}")
        else:
            ts_files = list(self.translations_dir.glob("*.ts"))
            if ts_files:
                cmd = ['linguist'] + [str(f) for f in ts_files]
                try:
                    subprocess.Popen(cmd, cwd=self.project_root)
                    print("✓ 已打开所有翻译文件")
                except FileNotFoundError:
                    print("✗ 找不到 linguist 命令，请确保 Qt 工具已安装")
            else:
                print("✗ 没有找到翻译文件")
    
    def show_help(self):
        """显示帮助信息"""
        help_text = """
Media Analyzer 统一国际化管理工具

用法:
    python i18n_manager.py [命令] [参数]

命令:
    update          更新翻译文件 (lupdate)
    release         生成翻译文件 (lrelease)
    complete-cn     补全中文翻译
    list            列出支持的语言
    linguist [语言] 打开Qt Linguist编辑器
    help            显示此帮助信息

示例:
    python i18n_manager.py update
    python i18n_manager.py release
    python i18n_manager.py complete-cn
    python i18n_manager.py linguist zh_CN
    python i18n_manager.py list

支持的语言代码:
    zh_CN  简体中文
    en_US  English
    ja_JP  日本語
    ko_KR  한국어
    fr_FR  Français
    de_DE  Deutsch
    es_ES  Español
    ru_RU  Русский
    ar_SA  العربية
    pt_BR  Português (Brasil)

完整工作流程:
    1. python i18n_manager.py update     # 更新翻译文件
    2. python i18n_manager.py complete-cn # 补全中文翻译
    3. python i18n_manager.py linguist zh_CN  # 编辑翻译
    4. python i18n_manager.py release     # 生成.qm文件
    5. 重新编译项目

功能说明:
    - 自动提取源代码中的tr()字符串
    - 智能过滤不需要翻译的内容
    - 支持批量翻译操作
    - 集成Qt标准工具
    - 包含完整的中文翻译映射
        """
        print(help_text)
    
    def run(self):
        """运行翻译管理器"""
        if len(sys.argv) < 2:
            self.show_help()
            return
        
        command = sys.argv[1].lower()
        
        if command == 'update':
            self.update_translations()
        elif command == 'release':
            self.release_translations()
        elif command == 'complete-cn':
            self.complete_chinese_translation()
        elif command == 'list':
            self.list_languages()
        elif command == 'linguist':
            lang_code = sys.argv[2] if len(sys.argv) > 2 else None
            self.open_linguist(lang_code)
        elif command == 'help' or command == '--help' or command == '-h':
            self.show_help()
        else:
            print(f"未知命令: {command}")
            self.show_help()

def main():
    if len(sys.argv) > 1 and not sys.argv[1].startswith('-'):
        project_root = sys.argv[1]
    else:
        project_root = Path(__file__).parent.parent
    
    manager = I18nManager(project_root)
    manager.run()

if __name__ == "__main__":
    main()