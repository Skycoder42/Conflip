TEMPLATE = app

QT += gui widgets datasync

TARGET = conflip
VERSION = $$CONFLIPVER

DEFINES += CONFLIP_LIBRARY

PUBLIC_HEADERS += \
	settingsplugin.h \
	libconflip_global.h \
	settingsfile.h \
	qsettingsplugin.h

HEADERS += $$PUBLIC_HEADERS \
	pluginloader.h \
    settingsdatabase.h

SOURCES += \
	settingsplugin.cpp \
	main.cpp \
	settingsfile.cpp \
	pluginloader.cpp \
	qsettingsplugin.cpp \
    settingsdatabase.cpp

unix {
	isEmpty(PREFIX): PREFIX = /usr

	target.path = $$PREFIX/bin

	tHeaders.path = $$PREFIX/include
	tHeaders.files = $$PUBLIC_HEADERS

	INSTALLS += target tHeaders
}
