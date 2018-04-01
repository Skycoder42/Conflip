TEMPLATE = lib

CONFIG += plugin

QT = core jsonserializer

TARGET = dconfsync
DESTDIR = $$OUT_PWD/../conflip

HEADERS += \
	dconfsynchelperplugin.h \
	dconfaccess.h \
	dconfsynchelper.h

SOURCES += \
	dconfsynchelperplugin.cpp \
	dconfaccess.cpp \
	dconfsynchelper.cpp

DISTFILES += dconfsync.json

include(../../lib.pri)

CONFIG += link_pkgconfig
PKGCONFIG += dconf
