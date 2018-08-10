TEMPLATE = app

QT = core service jsonserializer

CONFIG += console
CONFIG -= app_bundle

TARGET = $${TARGET_BASE}d
DESTDIR = $$BIN_DESTDIR

HEADERS += \
	syncengine.h \
	pathresolver.h \
	conflipservice.h

SOURCES += \
	main.cpp \
	syncengine.cpp \
	pathresolver.cpp \
	conflipservice.cpp

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

	TEST_TARGET=$${TARGET_BASE}@test
	slice_rundummy.target = $$BIN_DESTDIR/$$TEST_TARGET
	slice_rundummy.commands += echo \'$${LITERAL_HASH}!/bin/sh\' > $$shell_quote($$BIN_DESTDIR/$$TEST_TARGET) \
		$$escape_expand(\n\t)echo exec $$shell_quote($$BIN_DESTDIR/$$TARGET) $$shell_quote(\"$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}@\") --slice test >> $$shell_quote($$BIN_DESTDIR/$$TEST_TARGET) \
		$$escape_expand(\n\t)chmod a+x $$shell_quote($$BIN_DESTDIR/$$TEST_TARGET)

	QMAKE_EXTRA_TARGETS += create_service create_service_slice slice_rundummy
	PRE_TARGETDEPS += conflip.service conflip@.service "$$BIN_DESTDIR/$$TEST_TARGET"

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
