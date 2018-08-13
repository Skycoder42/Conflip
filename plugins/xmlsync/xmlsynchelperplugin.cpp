#include "xmlsynchelperplugin.h"
#include "xmlsynchelper.h"

XmlSyncHelperPlugin::XmlSyncHelperPlugin(QObject *parent) :
	QObject(parent)
{}

QStringList XmlSyncHelperPlugin::translations() const
{
	return {QStringLiteral("conflip_xmlsync")};
}

SyncHelper *XmlSyncHelperPlugin::createInstance(const QString &provider, QObject *parent)
{
	if(provider == XmlSyncHelper::ModeXml)
		return new XmlSyncHelper(parent);
	else
		return nullptr;
}
