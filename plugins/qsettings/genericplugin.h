#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include <settingsplugin.h>

class GenericPlugin : public SettingsPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID SettingsPlugin_iid FILE "qsettings.json")

public:
	GenericPlugin(QObject *parent = nullptr);

	int baum() const override;
};

#endif // GENERICPLUGIN_H
