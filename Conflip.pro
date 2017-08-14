TEMPLATE = subdirs

CONFIG += ordered
SUBDIRS += \
	conflip \
	plugins

DISTFILES += README.md \
	docker-compose.yaml

qpmlupdate.target = lupdate
qpmlupdate.CONFIG += recursive
qpmlupdate.recurse_target = lupdate

qpmlrelease.target = lrelease
qpmlrelease.CONFIG += recursive
qpmlrelease.recurse_target = lrelease

QMAKE_EXTRA_TARGETS += qpmlupdate qpmlrelease
