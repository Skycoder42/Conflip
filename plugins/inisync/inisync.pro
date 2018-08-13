TEMPLATE = lib

CONFIG += plugin

QT = core

TARGET = inisync
DESTDIR = $$PLUGIN_DESTDIR/conflip

HEADERS += \
	inisynchelperplugin.h \
	inisynchelper.h

SOURCES += \
	inisynchelperplugin.cpp \
	inisynchelper.cpp

TRANSLATIONS += conflip_inisync_de.ts \
	conflip_inisync_template.ts

DISTFILES += inisync.json

target.path = $$INSTALL_PLUGINS/conflip
qpmx_ts_target.path = $$INSTALL_TRANSLATIONS
INSTALLS += target qpmx_ts_target

include(../../lib.pri)

!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)
