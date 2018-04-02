#include "xmlsynchelperplugin.h"
#include "xmlsynchelper.h"

XmlSyncHelperPlugin::XmlSyncHelperPlugin(QObject *parent) :
	QObject(parent)
{}

SyncHelper *XmlSyncHelperPlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == XmlSyncHelper::ModeXml)
		return new XmlSyncHelper(parent);
	else
		return nullptr;
}
