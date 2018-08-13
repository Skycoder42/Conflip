TEMPLATE = lib

CONFIG += plugin

QT = core

TARGET = pathsync
DESTDIR = $$PLUGIN_DESTDIR/conflip

HEADERS += \
	pathsynchelperplugin.h \
	pathsynchelper.h

SOURCES += \
	pathsynchelperplugin.cpp \
	pathsynchelper.cpp

DISTFILES += pathsync.json

TRANSLATIONS += conflip_pathsync_de.ts \
	conflip_pathsync_template.ts

target.path = $$INSTALL_PLUGINS/conflip
qpmx_ts_target.path = $$INSTALL_TRANSLATIONS
INSTALLS += target qpmx_ts_target

include(../../lib.pri)

!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)
