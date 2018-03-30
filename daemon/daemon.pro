TEMPLATE = app

QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = conflipd

SOURCES += \
		main.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../lib/release/ -lconflip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../lib/debug/ -lconflip
else:unix: LIBS += -L$$OUT_PWD/../lib/ -lconflip

INCLUDEPATH += $$PWD/../lib
DEPENDPATH += $$PWD/../lib
