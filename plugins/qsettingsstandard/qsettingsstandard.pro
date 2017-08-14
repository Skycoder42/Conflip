TEMPLATE = lib

CONFIG += plugin
QT       += core gui widgets

TARGET = qsettings
VERSION = $$CONFLIPVER

DESTDIR = $$OUT_PWD/..

HEADERS += \
	qsettingsstandardplugin.h

SOURCES += \
	qsettingsstandardplugin.cpp

DISTFILES += qsettingsstandard.json \
	conflip_qsettings_standard_de.ts \
	conflip_qsettings_standard_template.ts

TRANSLATIONS += conflip_qsettings_standard_de.ts \
	conflip_qsettings_standard_template.ts

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../conflip/release/ -l:conflip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../conflip/debug/ -l:conflip
else:unix: LIBS += -L$$OUT_PWD/../../conflip/ -l:conflip

INCLUDEPATH += $$PWD/../../conflip
DEPENDPATH += $$PWD/../../conflip

unix {
	target.path = $$[QT_INSTALL_PLUGINS]/conflip
	INSTALLS += target
}

# include ts stuff
include(../../conflip/vendor/de/skycoder42/qpm-translate/de_skycoder42_qpm-translate.pri)
