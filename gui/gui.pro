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

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/release/ -lconflip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/debug/ -lconflip
else:unix: LIBS += -L$$OUT_PWD/../lib/ -lconflip

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib
