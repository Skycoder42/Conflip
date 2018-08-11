TEMPLATE = lib

CONFIG += plugin

QT = core

TARGET = jsonsync
DESTDIR = $$PLUGIN_DESTDIR/conflip

HEADERS += \
		jsonsynchelperplugin.h \
    jsonsynchelper.h

SOURCES += \
		jsonsynchelperplugin.cpp \
    jsonsynchelper.cpp

DISTFILES += jsonsync.json

target.path = $$INSTALL_PLUGINS/conflip
INSTALLS += target

include(../../lib.pri)
