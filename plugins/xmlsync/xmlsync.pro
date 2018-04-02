TEMPLATE = lib

CONFIG += plugin

QT = core xml

TARGET = xmlsync
DESTDIR = $$OUT_PWD/../conflip

HEADERS += \
		xmlsynchelperplugin.h \
	xmlsynchelper.h

SOURCES += \
		xmlsynchelperplugin.cpp \
	xmlsynchelper.cpp

DISTFILES += xmlsync.json

include(../../lib.pri)
