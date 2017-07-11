#include "pluginloader.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QGlobalStatic>
#ifndef QT_NO_DEBUG
#include <QCoreApplication>
#else
#include <QLibraryInfo>
#endif

Q_GLOBAL_STATIC(PluginLoader, pluginLoader)

PluginLoader::PluginLoader(QObject *parent) :
	QObject(parent),
	_plugins()
{
#ifndef QT_NO_DEBUG
	QDir plugDir(QCoreApplication::applicationDirPath());
	plugDir.cd(QStringLiteral("../plugins"));
#else
	QDir plugDir(QLibraryInfo::location(QLibraryInfo::PluginsPath));
	plugDir.cd(QStringLiteral("conflip"));
#endif

	plugDir.setFilter(QDir::Files | QDir::Readable);
	foreach (auto plg, plugDir.entryList()) {
		auto loader = new QPluginLoader(plugDir.absoluteFilePath(plg), this);
		auto meta = loader->metaData();
		if(meta[QStringLiteral("IID")].toString() == QStringLiteral(SettingsPlugin_iid)) {
			if(loader->load()) {
				auto keys = meta[QStringLiteral("MetaData")].toObject()[QStringLiteral("Keys")].toArray();
				foreach (auto key, keys)
					_plugins.insert(key.toString(), loader);
				if(!keys.isEmpty())
					continue;
				qWarning() << "Plugin" << plg << "has no associated keys";
			} else
				qWarning() << "Failed to load plugin" << plg << "with error" << loader->errorString();
		} else
			qWarning() << "File" << plg << "is not a conflip plugin";

		loader->deleteLater();
	}
}

void PluginLoader::loadPlugins()
{
	Q_UNUSED(pluginLoader)
}

QStringList PluginLoader::availablePlugins()
{
	return pluginLoader->_plugins.keys();
}
