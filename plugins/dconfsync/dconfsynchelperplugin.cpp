#include "dconfsynchelperplugin.h"
#include "dconfsynchelper.h"

DConfSyncHelperPlugin::DConfSyncHelperPlugin(QObject *parent) :
	QObject(parent)
{
	QJsonSerializer::registerAllConverters<DConfEntry>();
}

SyncHelper *DConfSyncHelperPlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == DConfSyncHelper::ModeDConf)
		return new DConfSyncHelper(parent);
	else
		return nullptr;
}
