TEMPLATE = lib

CONFIG += plugin

QT = core xml

TARGET = xmlsync
DESTDIR = $$PLUGIN_DESTDIR/conflip

HEADERS += \
	xmlsynchelperplugin.h \
	xmlsynchelper.h

SOURCES += \
	xmlsynchelperplugin.cpp \
	xmlsynchelper.cpp

TRANSLATIONS += conflip_xmlsync_de.ts \
	conflip_xmlsync_template.ts

DISTFILES += xmlsync.json

target.path = $$INSTALL_PLUGINS/conflip
qpmx_ts_target.path = $$INSTALL_TRANSLATIONS
INSTALLS += target qpmx_ts_target

include(../../lib.pri)

!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)
