TEMPLATE = subdirs

SUBDIRS += \
	lib \
	plugins \
	daemon \
	gui

plugins.depends += lib
daemon.depends += lib plugins
gui.depends += lib

DISTFILES += .qmake.conf
