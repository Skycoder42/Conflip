TEMPLATE = lib

QT += jsonserializer mvvmcore

TARGET = $$qtLibraryTarget($$TARGET_BASE)

DEFINES += CONFLIP_LIBRARY

HEADERS += \
	conflip.h \
	lib_conflip_global.h \
	syncentry.h \
	conflipdatabase.h \
	synchelper.h \
	synchelperplugin.h

SOURCES += \
	conflip.cpp \
	syncentry.cpp \
	conflipdatabase.cpp \
	synchelper.cpp

SETTINGS_DEFINITIONS += \
	settings.xml

DISTFILES += qpmx.json \
	conflip.pc.in

QMAKE_EXTRA_TARGETS += lrelease

!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)

# install copy
for(header, SETTINGSGENERATOR_BUILD_HEADERS) {
	theader = $$shadowed($$SETTINGSGENERATOR_DIR/$$basename(header))
	!exists($$theader):system($$QMAKE_COPY_FILE $$shell_quote($$shell_path($$header)) $$shell_quote($$shell_path($$theader)))
}

# install
linux {
	create_pc.target = conflip.pc
	create_pc.depends += $$PWD/conflip.pc.in
	create_pc.commands += sed "s:%{PREFIX}:$$INSTALL_PREFIX:g" $$PWD/conflip.pc.in | sed "s:%{VERSION}:$$VERSION:g" > conflip.pc

	QMAKE_EXTRA_TARGETS += create_pc
	PRE_TARGETDEPS += conflip.pc

	install_pc.files += $$OUT_PWD/conflip.pc
	install_pc.CONFIG += no_check_exist
	install_pc.path = $$INSTALL_LIBS/pkgconfig/
	INSTALLS += install_pc
}

target.path = $$INSTALL_LIBS
header_install.files = $$HEADERS $$SETTINGSGENERATOR_BUILD_HEADERS $$SETTINGSGENERATOR_DIR/settings.h
header_install.path = $$INSTALL_HEADERS/conflip
INSTALLS += target header_install
