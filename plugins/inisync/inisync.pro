TEMPLATE = lib

CONFIG += plugin

QT = core

TARGET = inisync
DESTDIR = $$OUT_PWD/../conflip

HEADERS += \
	inisynchelperplugin.h \
	inisynchelper.h

SOURCES += \
	inisynchelperplugin.cpp \
	inisynchelper.cpp

DISTFILES += inisync.json

include(../../lib.pri)
