#ifndef XMLSYNCHELPERPLUGIN_H
#define XMLSYNCHELPERPLUGIN_H

#include <synchelperplugin.h>

class XmlSyncHelperPlugin : public QObject, public SyncHelperPlugin
{
	Q_OBJECT
	Q_INTERFACES(SyncHelperPlugin)
	Q_PLUGIN_METADATA(IID SyncHelperPluginIid FILE "xmlsync.json")

public:
	XmlSyncHelperPlugin(QObject *parent = nullptr);

	SyncHelper *createInstance(const QString &provider, QObject *parent) override;
};

#endif // XMLSYNCHELPERPLUGIN_H
