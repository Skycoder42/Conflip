#ifndef DCONFSYNCHELPERPLUGIN_H
#define DCONFSYNCHELPERPLUGIN_H

#include <synchelperplugin.h>

class DConfSyncHelperPlugin : public QObject, public SyncHelperPlugin
{
	Q_OBJECT
	Q_INTERFACES(SyncHelperPlugin)
	Q_PLUGIN_METADATA(IID SyncHelperPluginIid FILE "dconfsync.json")

public:
	DConfSyncHelperPlugin(QObject *parent = nullptr);

	SyncHelper *createInstance(const QString &provider, QObject *parent) override;
};

#endif // DCONFSYNCHELPERPLUGIN_H
