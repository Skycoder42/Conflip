TEMPLATE = app

QT += core gui widgets

TARGET = conflip

HEADERS += \
		mainwindow.h

SOURCES += \
		main.cpp \
		mainwindow.cpp

FORMS += \
		mainwindow.ui

DISTFILES += qpmx.json

target.path = $$INSTALL_BINS
INSTALLS += target

include(../lib.pri)

!ReleaseBuild:!DebugBuild:!system(qpmx -d $$shell_quote($$_PRO_FILE_PWD_) --qmake-run init $$QPMX_EXTRA_OPTIONS $$shell_quote($$QMAKE_QMAKE) $$shell_quote($$OUT_PWD)): error(qpmx initialization failed. Check the compilation log for details.)
else: include($$OUT_PWD/qpmx_generated.pri)
