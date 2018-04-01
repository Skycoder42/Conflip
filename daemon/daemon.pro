TEMPLATE = app

QT += jsonserializer
QT -= gui

CONFIG += console
CONFIG -= app_bundle

TARGET = conflipd

HEADERS += \
	syncengine.h \
	pathresolver.h

SOURCES += \
	main.cpp \
	syncengine.cpp \
	pathresolver.cpp

DISTFILES += \
	conflip.service.in

# install
linux {
	# TODO add slice and system service variants
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
include(../lib.pri)

!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)
