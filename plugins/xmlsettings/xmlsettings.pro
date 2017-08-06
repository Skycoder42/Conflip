TEMPLATE = lib

CONFIG += plugin
QT       += core gui widgets xml

TARGET = xml

DESTDIR = $$OUT_PWD/..

HEADERS += \
	xmlsettingsplugin.h \
	xmlsettingsfile.h

SOURCES += \
	xmlsettingsplugin.cpp \
	xmlsettingsfile.cpp

DISTFILES += xmlsettings.json

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../conflip/release/ -l:conflip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../conflip/debug/ -l:conflip
else:unix: LIBS += -L$$OUT_PWD/../../conflip/ -l:conflip

INCLUDEPATH += $$PWD/../../conflip
DEPENDPATH += $$PWD/../../conflip

unix {
	target.path = $$[QT_INSTALL_PLUGINS]/conflip
	INSTALLS += target
}