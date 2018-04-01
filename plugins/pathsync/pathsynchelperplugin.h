#ifndef PATHSYNCHELPERPLUGIN_H
#define PATHSYNCHELPERPLUGIN_H

#include <synchelperplugin.h>

class PathSyncHelperPlugin : public QObject, public SyncHelperPlugin
{
	Q_OBJECT
	Q_INTERFACES(SyncHelperPlugin)
	Q_PLUGIN_METADATA(IID SyncHelperPluginIid FILE "pathsync.json")

public:
	PathSyncHelperPlugin(QObject *parent = nullptr);

	SyncHelper *createInstance(const QString &provider, QObject *parent) override;
};

#endif // PATHSYNCHELPERPLUGIN_H
