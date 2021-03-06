TEMPLATE = subdirs

SUBDIRS += \
	pathsync \
	inisync \
	xmlsync \
	jsonsync

unix:!android:!ios:system(pkg-config --exists dconf): SUBDIRS += dconfsync

prepareRecursiveTarget(lrelease)
QMAKE_EXTRA_TARGETS += lrelease
