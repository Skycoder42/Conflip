#include "qsettingsfile.h"
#include "qsettingsplugin.h"

QSettingsPlugin::QSettingsPlugin(QObject *parent) :
	SettingsPlugin(parent)
{}

SettingsFile *QSettingsPlugin::createSettings(const QString &path, const QString &type, QObject *parent)
{
	if(!_formats.contains(type))
		_formats.insert(type, registerFormat(type));

	auto format = _formats.value(type);
	if(format == QSettings::InvalidFormat)
		return nullptr;
	else
		return new QSettingsFile(path, format, parent);//TODO return QSettingsFile
}
