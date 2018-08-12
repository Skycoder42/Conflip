#include "jsonsynchelperplugin.h"
#include "jsonsynchelper.h"

JsonSyncHelperPlugin::JsonSyncHelperPlugin(QObject *parent) :
	QObject{parent}
{}

SyncHelper *JsonSyncHelperPlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == JsonSyncHelper::ModeJson ||
	   provider == JsonSyncHelper::ModeQbjs)
		return new JsonSyncHelper{parent};
	else
		return nullptr;
}
