#include "inisynchelperplugin.h"
#include "inisynchelper.h"

IniSyncHelperPlugin::IniSyncHelperPlugin(QObject *parent) :
	QObject(parent)
{}

SyncHelper *IniSyncHelperPlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == IniSyncHelper::ModeIni)
		return new IniSyncHelper(parent);
	else
		return nullptr;
}
