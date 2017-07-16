#include "xmlsettingsfile.h"
#include "xmlsettingsplugin.h"

XmlSettingsPlugin::XmlSettingsPlugin(QObject *parent) :
	SettingsPlugin(parent)
{}

SettingsFile *XmlSettingsPlugin::createSettings(const QString &path, const QString &type, QObject *parent)
{
	if(type == QStringLiteral("xml"))
		return new XmlSettingsFile(path, parent);
	else
		throw SettingsLoadException("Type does not name a valid xml format");
}

QString XmlSettingsPlugin::displayName(const QString &type) const
{
	if(type == QStringLiteral("xml"))
		return tr("XML-Files (*.xml)");
	else
		return QString();
}
