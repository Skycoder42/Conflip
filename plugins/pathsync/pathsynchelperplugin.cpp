#include "pathsynchelperplugin.h"
#include "pathsynchelper.h"

PathSyncHelperPlugin::PathSyncHelperPlugin(QObject *parent) :
	QObject{parent}
{}

QStringList PathSyncHelperPlugin::translations() const
{
	return {QStringLiteral("conflip_pathsync")};
}

SyncHelper *PathSyncHelperPlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == PathSyncHelper::ModeSymlink || provider == PathSyncHelper::ModeCopy)
		return new PathSyncHelper{parent};
	else
		return nullptr;
}
