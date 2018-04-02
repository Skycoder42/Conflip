#include "conflip.h"
#include <QCoreApplication>
#include <QJsonSerializer>
#include <qpluginfactory.h>
#include "syncentry.h"
#include "synchelperplugin.h"

namespace {

void conflip_lib_startup()
{
	QJsonSerializer::registerAllConverters<SyncEntry>();
}

}
Q_COREAPP_STARTUP_FUNCTION(conflip_lib_startup)

Q_GLOBAL_PLUGIN_OBJECT_FACTORY(SyncHelperPlugin, SyncHelper, "conflip", helperFactory)

QString Conflip::ConfigFileName()
{
	return QStringLiteral("config.json");
}

QStringList Conflip::listPlugins()
{
	return helperFactory->allKeys();
}

SyncHelper *Conflip::loadHelper(const QString &type, QObject *parent)
{
	return helperFactory->createInstance(type, parent);
}

