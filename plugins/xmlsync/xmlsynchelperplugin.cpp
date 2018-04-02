#include "xmlsynchelperplugin.h"
#include "xmlsynchelper.h"

XmlSyncHelperPlugin::XmlSyncHelperPlugin(QObject *parent) :
	QObject(parent)
{}

SyncHelper *XmlSyncHelperPlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == XmlSyncHelper::XmlMode)
		return new XmlSyncHelper(parent);
	else
		return nullptr;
}
