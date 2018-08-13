#include "dconfsynchelperplugin.h"
#include "dconfsynchelper.h"

DConfSyncHelperPlugin::DConfSyncHelperPlugin(QObject *parent) :
	QObject(parent)
{
	QJsonSerializer::registerAllConverters<DConfEntry>();
}

QStringList DConfSyncHelperPlugin::translations() const
{
	return {QStringLiteral("conflip_dconfsync")};
}

SyncHelper *DConfSyncHelperPlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == DConfSyncHelper::ModeDConf)
		return new DConfSyncHelper(parent);
	else
		return nullptr;
}
