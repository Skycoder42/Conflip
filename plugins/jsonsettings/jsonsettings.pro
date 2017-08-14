TEMPLATE = lib

CONFIG += plugin
QT       += core gui widgets

TARGET = json

DESTDIR = $$OUT_PWD/..

HEADERS += \
	jsonsettingsplugin.h \
	jsonsettingsfile.h

SOURCES += \
	jsonsettingsplugin.cpp \
	jsonsettingsfile.cpp

DISTFILES += jsonsettings.json \
	conflip_json_de.ts \
	conflip_json_template.ts

TRANSLATIONS += conflip_json_de.ts \
	conflip_json_template.ts

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../conflip/release/ -l:conflip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../conflip/debug/ -l:conflip
else:unix: LIBS += -L$$OUT_PWD/../../conflip/ -l:conflip

INCLUDEPATH += $$PWD/../../conflip
DEPENDPATH += $$PWD/../../conflip

unix {
	target.path = $$[QT_INSTALL_PLUGINS]/conflip

	trInstall.path = $$[QT_INSTALL_TRANSLATIONS]
	trInstall.files = $$OUT_PWD/conflip_json_de.qm \
		$$PWD/conflip_json_template.ts
	trInstall.CONFIG += no_check_exist

	INSTALLS += target trInstall
}

# include ts stuff
include(../../conflip/vendor/de/skycoder42/qpm-translate/de_skycoder42_qpm-translate.pri)
