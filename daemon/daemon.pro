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
	conflip.service.in \
	qpmx.json

# install
linux {
	create_service.target = conflip.service
	create_service.depends += $$PWD/conflip.service.in
	create_service.commands += sed "s:%{INSTALL_BINS}:$$INSTALL_BINS:g" $$PWD/conflip.service.in | sed "s:%{SLICE}::g" > conflip.service

	create_service_slice.target = conflip@.service
	create_service_slice.depends += $$PWD/conflip.service.in
	create_service_slice.commands += sed "s:%{INSTALL_BINS}:$$INSTALL_BINS:g" $$PWD/conflip.service.in | sed $$shell_quote(s:%{SLICE}:--slice %I:g) > conflip@.service

	QMAKE_EXTRA_TARGETS += create_service create_service_slice
	PRE_TARGETDEPS += conflip.service conflip@.service

	install_service_user.files += $$OUT_PWD/conflip.service $$OUT_PWD/conflip@.service
	install_service_user.CONFIG += no_check_exist
	install_service_user.path = $$INSTALL_LIBS/systemd/user/
	install_service_system.files += $$OUT_PWD/conflip.service $$OUT_PWD/conflip@.service
	install_service_system.CONFIG += no_check_exist
	install_service_system.path = $$INSTALL_LIBS/systemd/system/
	INSTALLS += install_service_user install_service_system
}

target.path = $$INSTALL_BINS
INSTALLS += target

# libs
include(../lib.pri)

!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)
