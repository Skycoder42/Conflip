TEMPLATE = app

QT += gui widgets datasync

TARGET = conflip
VERSION = $$CONFLIPVER

QMAKE_TARGET_COMPANY = "Skycoder42"
QMAKE_TARGET_PRODUCT = $$TARGET
QMAKE_TARGET_DESCRIPTION = "Conflip"
QMAKE_TARGET_COPYRIGHT = "Felix Barz"
QMAKE_TARGET_BUNDLE_PREFIX = de.skycoder42

DEFINES += "TARGET=\\\"$$TARGET\\\""
DEFINES += "VERSION=\\\"$$VERSION\\\""
DEFINES += "COMPANY=\"\\\"$$QMAKE_TARGET_COMPANY\\\"\""
DEFINES += "DISPLAY_NAME=\"\\\"$$QMAKE_TARGET_DESCRIPTION\\\"\""
DEFINES += "BUNDLE=\"\\\"$$QMAKE_TARGET_BUNDLE_PREFIX\\\"\""

DEFINES += CONFLIP_LIBRARY

include(vendor/vendor.pri)

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
    datastore.h

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
    datastore.cpp

FORMS += \
	editsettingsobjectdialog.ui

unix {
	isEmpty(PREFIX): PREFIX = /usr

	target.path = $$PREFIX/bin

	tHeaders.path = $$PREFIX/include
	tHeaders.files = $$PUBLIC_HEADERS

	INSTALLS += target tHeaders
}
