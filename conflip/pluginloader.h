#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include "libconflip_global.h"
#include <QObject>
#include <QPluginLoader>
#include "settingsplugin.h"

class PluginLoader : public QObject
{
	Q_OBJECT

public:
	explicit PluginLoader(QObject *parent = nullptr);

	static void loadPlugins();

	static QStringList availablePlugins();
	static SettingsFile *createSettings(const QString &path, const QString &type, QObject *parent = nullptr);

private:
	QHash<QString, QPluginLoader*> _plugins;
};

#endif // PLUGINLOADER_H
