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

QString QSettingsStandardPlugin::displayName(const QString &type) const
{
	if(type == QStringLiteral("native"))
#if defined(Q_OS_LINUX)
		return tr("Config-Files (*.conf)");
#elif defined(Q_OS_WIN)
		return tr("Registry-Entries or Registry-Files (*.reg)");
#elif defined(Q_OS_MAC)
		return tr("PList-Files (*.plist)");
#endif
	else if(type == QStringLiteral("ini"))
		return tr("Ini-Files (*.ini)");
	else
		return QString();
}
