CFLIB_DIR = $$shadowed($$PWD/lib)

win32:CONFIG(release, debug|release): LIBS += -L$$CFLIB_DIR/release/ -lconflip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$CFLIB_DIR/debug/ -lconflip
else:unix: LIBS += -L$$CFLIB_DIR/ -lconflip

INCLUDEPATH += $$PWD/lib $$CFLIB_DIR
DEPENDPATH += $$PWD/lib
