TEMPLATE = subdirs

SUBDIRS += \
	lib \
	plugins \
	daemon \
	gui

plugins.depends += lib
daemon.depends += lib plugins
gui.depends += lib

lrelease.target = lrelease
lrelease.CONFIG = recursive
lrelease.recurse_target = lrelease
lrelease.recurse += gui lib
QMAKE_EXTRA_TARGETS += lrelease

DISTFILES += .qmake.conf
