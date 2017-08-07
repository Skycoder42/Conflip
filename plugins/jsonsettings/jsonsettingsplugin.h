#ifndef JSONSETTINGSPLUGIN_H
#define JSONSETTINGSPLUGIN_H

#include <settingsplugin.h>

class JsonSettingsPlugin : public SettingsPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID SettingsPlugin_iid FILE "jsonsettings.json")

public:
	JsonSettingsPlugin(QObject *parent = nullptr);

	SettingsFile *createSettings(const QString &path, const QString &type, QObject *parent) override;
	QString displayName(const QString &type) const override;
	QStringList fileFilters(const QString &type) const override;
};

#endif // JSONSETTINGSPLUGIN_H
