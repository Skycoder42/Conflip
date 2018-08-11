#include "jsonsynchelperplugin.h"
#include "jsonsynchelper.h"

JsonSyncHelperPlugin::JsonSyncHelperPlugin(QObject *parent) :
	QObject{parent}
{}

SyncHelper *JsonSyncHelperPlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == JsonSyncHelper::ModeJson)
		return new JsonSyncHelper{false, parent};
	else if(provider == JsonSyncHelper::ModeQbjs)
		return new JsonSyncHelper{true, parent};
	else
		return nullptr;
}
