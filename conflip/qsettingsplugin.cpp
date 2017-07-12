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
		throw SettingsLoadException("Type does not name a valid QSettings format");
	else {
		auto settings = new QSettings(path, format);
		if(settings->status() != QSettings::NoError)
			throw SettingsLoadException(settings->status() == QSettings::AccessError ?
											"Access Denied" :
											"Malformed File");
		return new QSettingsFile(settings, parent);
	}
}
