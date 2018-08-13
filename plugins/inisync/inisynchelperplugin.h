#ifndef INISYNCHELPERPLUGIN_H
#define INISYNCHELPERPLUGIN_H

#include <synchelperplugin.h>

class IniSyncHelperPlugin : public QObject, public SyncHelperPlugin
{
	Q_OBJECT
	Q_INTERFACES(SyncHelperPlugin)
	Q_PLUGIN_METADATA(IID SyncHelperPluginIid FILE "inisync.json")

public:
	IniSyncHelperPlugin(QObject *parent = nullptr);

	QStringList translations() const override;
	SyncHelper *createInstance(const QString &provider, QObject *parent) override;
};

#endif // INISYNCHELPERPLUGIN_H
