#ifndef XMLSETTINGSPLUGIN_H
#define XMLSETTINGSPLUGIN_H

#include <settingsplugin.h>

class XmlSettingsPlugin : public SettingsPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID SettingsPlugin_iid FILE "xmlsettings.json")

public:
	XmlSettingsPlugin(QObject *parent = nullptr);

	SettingsFile *createSettings(const QString &path, const QString &type, QObject *parent) override;
	QString displayName(const QString &type) const override;
	QStringList fileFilters(const QString &type) const override;
};

#endif // XMLSETTINGSPLUGIN_H
