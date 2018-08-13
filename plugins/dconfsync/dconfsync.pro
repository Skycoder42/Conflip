TEMPLATE = lib

CONFIG += plugin

QT = core jsonserializer

TARGET = dconfsync
DESTDIR = $$PLUGIN_DESTDIR/conflip

HEADERS += \
	dconfsynchelperplugin.h \
	dconfaccess.h \
	dconfsynchelper.h

SOURCES += \
	dconfsynchelperplugin.cpp \
	dconfaccess.cpp \
	dconfsynchelper.cpp

TRANSLATIONS += conflip_dconfsync_de.ts \
	conflip_dconfsync_template.ts

DISTFILES += dconfsync.json

target.path = $$INSTALL_PLUGINS/conflip
qpmx_ts_target.path = $$INSTALL_TRANSLATIONS
INSTALLS += target qpmx_ts_target

include(../../lib.pri)

CONFIG += link_pkgconfig
PKGCONFIG += dconf

!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)
