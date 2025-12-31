# SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
# SPDX-License-Identifier: MIT
dh_clean
rm *.o ui_* moc_* qrc_* Makefile MediaAnalyzer media-analyzer *.qrc .qmake.stash media-analyzer.debug media-analyzer-qt*
rm -rf app_logs/ .qm/ 
rm -rf debian/.debhelper/ debian/media-analyzer/ debian/files debian/debhelper-build-stamp debian/media-analyzer.debhelper.log debian/media-analyzer.substvars
rm -rf debian/*.log debian/*.substvars debian/media-analyzer-dde-filemanager-contextmenu/ debian/media-analyzer-nautilus-contextmenu/ debian/media-analyzer-dbus
rm -rf build-qt5/ build-qt6/
clear
ls -al
