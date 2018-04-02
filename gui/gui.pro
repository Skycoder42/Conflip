TEMPLATE = app

QT += core gui widgets jsonserializer

TARGET = conflip

HEADERS += \
		mainwindow.h \
	createentrydialog.h

SOURCES += \
		main.cpp \
		mainwindow.cpp \
	createentrydialog.cpp

FORMS += \
		mainwindow.ui \
	createentrydialog.ui

DISTFILES += qpmx.json \
	conflip.svg \
	conflip.desktop

target.path = $$INSTALL_BINS
install_icons.files += conflip.svg
install_icons.path = $$INSTALL_SHARE/icons/hicolor/scalable/apps
install_desktop.files = conflip.desktop
install_desktop.path = $$INSTALL_SHARE/applications/
INSTALLS += target install_icons install_desktop

include(../lib.pri)

!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)
