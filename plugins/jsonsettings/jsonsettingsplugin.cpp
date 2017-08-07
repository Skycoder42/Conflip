#include "jsonsettingsfile.h"
#include "jsonsettingsplugin.h"

JsonSettingsPlugin::JsonSettingsPlugin(QObject *parent) :
	SettingsPlugin(parent)
{}

SettingsFile *JsonSettingsPlugin::createSettings(const QString &path, const QString &type, QObject *parent)
{
	if(type == QStringLiteral("json"))
		return new JsonSettingsFile(path, false, parent);
	else if(type == QStringLiteral("qbjs"))
		return new JsonSettingsFile(path, true, parent);
	else
		throw SettingsLoadException("Type does not name a valid json format");
}

QString JsonSettingsPlugin::displayName(const QString &type) const
{
	if(type == QStringLiteral("json"))
		return tr("Json");
	else if(type == QStringLiteral("qbjs"))
		return tr("Binary Json (Qt)");
	else
		return QString();
}

QStringList JsonSettingsPlugin::fileFilters(const QString &type) const
{
	if(type == QStringLiteral("json"))
		return {tr("Json-Files (*.json)")};
	else if(type == QStringLiteral("qbjs"))
		return {tr("Qt Binary Json-Files (*.qbjs)")};
	else
		return QStringList();
}
