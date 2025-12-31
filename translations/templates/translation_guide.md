# Media Analyzer 翻译指南

## 概述
本文档说明如何为 Media Analyzer 项目贡献翻译。

## 支持的语言
- zh_CN: 简体中文
- en_US: English
- ja_JP: 日本語
- ko_KR: 한국어
- fr_FR: Français
- de_DE: Deutsch
- es_ES: Español
- ru_RU: Русский
- ar_SA: العربية
- pt_BR: Português (Brasil)

## 翻译流程

### 1. 准备工作
- 安装 Qt Linguist 工具
- 或者使用文本编辑器直接编辑 .ts 文件

### 2. 翻译文件位置
翻译文件位于 `translations/` 目录下，文件名格式为 `media-analyzer_语言代码.ts`

### 3. 翻译注意事项
- 保持专业术语的一致性
- 注意界面元素的长度限制
- 保留占位符如 %1, %2 等
- 考虑文化差异和本地化需求

### 4. 翻译完成后
- 运行 `lrelease` 生成 .qm 文件
- 测试翻译效果

## 自动化工具
使用 `tools/update_translations.py` 脚本可以：
- 自动提取源代码中的可翻译字符串
- 更新翻译文件
- 生成新的语言支持

## 联系方式
如有问题，请通过项目仓库提交 Issue。
