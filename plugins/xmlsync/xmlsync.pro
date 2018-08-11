TEMPLATE = lib

CONFIG += plugin

QT = core xml

TARGET = xmlsync
DESTDIR = $$PLUGIN_DESTDIR/conflip

HEADERS += \
		xmlsynchelperplugin.h \
	xmlsynchelper.h

SOURCES += \
		xmlsynchelperplugin.cpp \
	xmlsynchelper.cpp

DISTFILES += xmlsync.json

target.path = $$INSTALL_PLUGINS/conflip
INSTALLS += target

include(../../lib.pri)
