#include "inisynchelperplugin.h"
#include "inisynchelper.h"

IniSyncHelperPlugin::IniSyncHelperPlugin(QObject *parent) :
	QObject{parent}
{}

QStringList IniSyncHelperPlugin::translations() const
{
	return {QStringLiteral("conflip_inisync")};
}

SyncHelper *IniSyncHelperPlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == IniSyncHelper::ModeIni)
		return new IniSyncHelper(parent);
	else
		return nullptr;
}
