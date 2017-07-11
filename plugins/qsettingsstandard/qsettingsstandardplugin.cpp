#include "qsettingsstandardplugin.h"

QSettingsStandardPlugin::QSettingsStandardPlugin(QObject *parent) :
	SettingsPlugin(parent)
{}

SettingsFile *QSettingsStandardPlugin::createSettings(const QString &path, const QString &type, QObject *parent)
{
	return nullptr;
}
