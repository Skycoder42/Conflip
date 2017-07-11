TEMPLATE = app

QT += gui widgets

TARGET = conflip
VERSION = $$CONFLIPVER

DEFINES += CONFLIP_LIBRARY

PUBLIC_HEADERS += \
	settingsplugin.h \
	libconflip_global.h

PRIVATE_HEADERS +=

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERSQM

SOURCES += \
	settingsplugin.cpp \
	main.cpp

unix {
	isEmpty(PREFIX): PREFIX = /usr

	target.path = $$PREFIX/bin

	tHeaders.path = $$PREFIX/include
	tHeaders.files = $$PUBLIC_HEADERS

	INSTALLS += target tHeaders
}
