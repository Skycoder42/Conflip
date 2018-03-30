TEMPLATE = subdirs

SUBDIRS += \
	lib \
	daemon \
	gui

daemon.depends += lib
gui.depends += lib

DISTFILES += .qmake.conf
