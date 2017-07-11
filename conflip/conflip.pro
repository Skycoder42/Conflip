TEMPLATE = app

QT += gui widgets

TARGET = conflip
VERSION = $$CONFLIPVER

DEFINES += CONFLIP_LIBRARY

PUBLIC_HEADERS += \
	settingsplugin.h \
	libconflip_global.h \
	settingsfile.h

PRIVATE_HEADERS += \
	pluginloader.h

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

SOURCES += \
	settingsplugin.cpp \
	main.cpp \
	settingsfile.cpp \
	pluginloader.cpp

unix {
	isEmpty(PREFIX): PREFIX = /usr

	target.path = $$PREFIX/bin

	tHeaders.path = $$PREFIX/include
	tHeaders.files = $$PUBLIC_HEADERS

	INSTALLS += target tHeaders
}
