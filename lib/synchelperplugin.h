#ifndef SYNCHELPERPLUGIN_H
#define SYNCHELPERPLUGIN_H

#include <QObject>
#include "synchelper.h"

class SyncHelperPlugin
{
	Q_DISABLE_COPY(SyncHelperPlugin)
public:
	inline SyncHelperPlugin() = default;
	virtual inline ~SyncHelperPlugin() = default;

	virtual SyncHelper *createInstance(const QString &provider, QObject *parent = nullptr) = 0;
};

#define SyncHelperPluginIid "de.skycoder42.conflip.SyncHelperPlugin"
Q_DECLARE_INTERFACE(SyncHelperPlugin, SyncHelperPluginIid)

#endif // SYNCHELPERPLUGIN_H
