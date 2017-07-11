#include "qsettingsstandardplugin.h"

QSettingsStandardPlugin::QSettingsStandardPlugin(QObject *parent) :
	QSettingsPlugin(parent)
{}

QSettings::Format QSettingsStandardPlugin::registerFormat(const QString &type)
{
	if(type == QStringLiteral("native"))
		return QSettings::NativeFormat;
	else if(type == QStringLiteral("ini"))
		return QSettings::IniFormat;
	else
		return QSettings::InvalidFormat;
}
