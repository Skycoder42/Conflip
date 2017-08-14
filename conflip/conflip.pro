TEMPLATE = app

QT += gui widgets datasync

TARGET = conflip
VERSION = $$CONFLIPVER

QMAKE_TARGET_COMPANY = "Skycoder42"
QMAKE_TARGET_PRODUCT = $$TARGET
QMAKE_TARGET_DESCRIPTION = "Conflip"
QMAKE_TARGET_COPYRIGHT = "Felix Barz"
QMAKE_TARGET_BUNDLE_PREFIX = de.skycoder42

RC_ICONS += ./icons/conflip.ico
ICON = ./icons/conflip.icns

DEFINES += "TARGET=\\\"$$TARGET\\\""
DEFINES += "VERSION=\\\"$$VERSION\\\""
DEFINES += "COMPANY=\"\\\"$$QMAKE_TARGET_COMPANY\\\"\""
DEFINES += "DISPLAY_NAME=\"\\\"$$QMAKE_TARGET_DESCRIPTION\\\"\""
DEFINES += "BUNDLE=\"\\\"$$QMAKE_TARGET_BUNDLE_PREFIX\\\"\""

DEFINES += CONFLIP_LIBRARY

PUBLIC_HEADERS += \
	settingsplugin.h \
	libconflip_global.h \
	settingsfile.h \
	qsettingsplugin.h

HEADERS += $$PUBLIC_HEADERS \
	pluginloader.h \
	settingsdatabase.h \
	editsettingsobjectdialog.h \
	qsettingsfile.h \
	settingsfilemodel.h \
	datastore.h \
	traycontrol.h \
	managesettingsdialog.h \
	syncmanager.h \
	synclogger.h \
	changeremotedialog.h

SOURCES += \
	settingsplugin.cpp \
	main.cpp \
	settingsfile.cpp \
	pluginloader.cpp \
	qsettingsplugin.cpp \
	settingsdatabase.cpp \
	editsettingsobjectdialog.cpp \
	qsettingsfile.cpp \
	settingsfilemodel.cpp \
	datastore.cpp \
	traycontrol.cpp \
	managesettingsdialog.cpp \
	syncmanager.cpp \
	synclogger.cpp \
	changeremotedialog.cpp

FORMS += \
	editsettingsobjectdialog.ui \
	managesettingsdialog.ui \
	changeremotedialog.ui

DISTFILES += conflip_de.ts \
	conflip_template.ts \
	conflip.pc

TRANSLATIONS += conflip_de.ts \
	conflip_template.ts

include(vendor/vendor.pri)

unix {
	#DEBUG LD_LIBRARY_PATH
	LIBS += -L$$OUT_PWD

	isEmpty(PREFIX): PREFIX = /usr

	target.path = $$[QT_INSTALL_BINS]

	tHeaders.path = $$[QT_INSTALL_HEADERS]/../conflip
	tHeaders.files = $$PUBLIC_HEADERS

	pcFile.path = $$[QT_INSTALL_LIBS]/pkgconfig
	pcFile.files = conflip.pc

	trInstall.path = $$[QT_INSTALL_TRANSLATIONS]
	trInstall.files = $$OUT_PWD/conflip_de.qm \
		$$PWD/conflip_template.ts
	trInstall.CONFIG += no_check_exist

	INSTALLS += target tHeaders pcFile trInstall
}

RESOURCES += \
	conflip.qrc
