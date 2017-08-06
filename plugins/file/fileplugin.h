#ifndef FILEPLUGIN_H
#define FILEPLUGIN_H

#include <settingsplugin.h>

class FilePlugin : public SettingsPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID SettingsPlugin_iid FILE "file.json")

public:
	FilePlugin(QObject *parent = nullptr);

	SettingsFile *createSettings(const QString &path, const QString &type, QObject *parent) override;
	QString displayName(const QString &type) const override;
};

#endif // FILEPLUGIN_H
