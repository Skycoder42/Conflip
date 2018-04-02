TEMPLATE = lib

CONFIG += plugin

QT = core

TARGET = pathsync
DESTDIR = $$OUT_PWD/../conflip

HEADERS += \
	pathsynchelperplugin.h \
	pathsynchelper.h

SOURCES += \
	pathsynchelperplugin.cpp \
	pathsynchelper.cpp

DISTFILES += pathsync.json

target.path = $$INSTALL_PLUGINS/conflip
INSTALLS += target

include(../../lib.pri)
