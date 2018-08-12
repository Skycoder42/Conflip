#include "pathsynchelperplugin.h"
#include "pathsynchelper.h"

PathSyncHelperPlugin::PathSyncHelperPlugin(QObject *parent) :
	QObject{parent}
{}

SyncHelper *PathSyncHelperPlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == PathSyncHelper::ModeSymlink || provider == PathSyncHelper::ModeCopy)
		return new PathSyncHelper{parent};
	else
		return nullptr;
}
