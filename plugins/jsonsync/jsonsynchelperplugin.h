#ifndef JSONSYNCHELPERPLUGIN_H
#define JSONSYNCHELPERPLUGIN_H

#include <synchelperplugin.h>

class JsonSyncHelperPlugin : public QObject, public SyncHelperPlugin
{
	Q_OBJECT
	Q_INTERFACES(SyncHelperPlugin)
	Q_PLUGIN_METADATA(IID SyncHelperPluginIid FILE "jsonsync.json")

public:
	JsonSyncHelperPlugin(QObject *parent = nullptr);

	QStringList translations() const override;
	SyncHelper *createInstance(const QString &provider, QObject *parent) override;
};

#endif // JSONSYNCHELPERPLUGIN_H
