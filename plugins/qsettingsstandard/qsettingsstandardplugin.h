#ifndef QSETTINGSSTANDARDPLUGIN_H
#define QSETTINGSSTANDARDPLUGIN_H

#include <settingsplugin.h>

class QSettingsStandardPlugin : public SettingsPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID SettingsPlugin_iid FILE "qsettingsstandard.json")

public:
	QSettingsStandardPlugin(QObject *parent = nullptr);

	SettingsFile *createSettings(const QString &path, const QString &type, QObject *parent) override;
};

#endif // QSETTINGSSTANDARDPLUGIN_H
