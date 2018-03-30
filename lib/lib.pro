TEMPLATE = lib

QT += jsonserializer
QT -= gui

TARGET = conflip

DEFINES += CONFLIP_LIBRARY

HEADERS += \
	conflip.h \
	lib_conflip_global.h \
    syncentry.h \
    conflipdatabase.h

SOURCES += \
	conflip.cpp \
    syncentry.cpp \
    conflipdatabase.cpp

SETTINGS_GENERATORS += \
	settings.xml

DISTFILES += qpmx.json

!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)

# install copy
for(header, SETTINGSGENERATOR_BUILD_HEADERS) {
	theader = $$shadowed($$SETTINGSGENERATOR_DIR/$$basename(header))
	!exists($$theader):system($$QMAKE_COPY_FILE $$shell_quote($$shell_path($$header)) $$shell_quote($$shell_path($$theader)))
}

