TEMPLATE = app

QT += jsonserializer
QT -= gui

CONFIG += console
CONFIG -= app_bundle

TARGET = conflipd

SOURCES += \
		main.cpp \
	syncengine.cpp \
    pathresolver.cpp

DISTFILES += \
	conflip.service.in

# install
linux {
	create_service.target = conflip.service
	create_service.depends += $$PWD/conflip.service.in
	create_service.commands += sed "s:%{INSTALL_BINS}:$$INSTALL_BINS:g" $$PWD/conflip.service.in > conflip.service

	QMAKE_EXTRA_TARGETS += create_service
	PRE_TARGETDEPS += conflip.service

	install_service.files += $$OUT_PWD/conflip.service
	install_service.CONFIG += no_check_exist
	install_service.path = $$INSTALL_LIBS/systemd/user/
	INSTALLS += install_service
}

# libs
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/release/ -lconflip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/debug/ -lconflip
else:unix: LIBS += -L$$OUT_PWD/../lib/ -lconflip

INCLUDEPATH += $$PWD/../lib $$OUT_PWD/../lib
DEPENDPATH += $$PWD/../lib

HEADERS += \
	syncengine.h \
    pathresolver.h
