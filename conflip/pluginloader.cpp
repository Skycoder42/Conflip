#include "pluginloader.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonValue>
#include <QLibraryInfo>
#include <QTranslator>
#include <QDebug>
#include <QGlobalStatic>
#include <QCoreApplication>

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
				auto instance = qobject_cast<SettingsPlugin*>(loader->instance());
				if(instance) {
					auto metadata = meta[QStringLiteral("MetaData")].toObject();
					auto keys = metadata[QStringLiteral("Keys")].toArray();
					foreach (auto key, keys)
						_plugins.insert(key.toString(), instance);

					auto ts = metadata[QStringLiteral("Translations")].toString();
					if(!ts.isEmpty()) {
						auto translator = new QTranslator(instance);
						if(translator->load(QLocale(),
											ts,
											QStringLiteral("_"),
											QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
							qApp->installTranslator(translator);
						else {
							qWarning() << "Failed to load translations for" << ts;
							delete translator;
						}
					}

					if(!keys.isEmpty())
						continue;
					qWarning() << "Plugin" << plg << "has no associated keys";
				} else
					qWarning() << "File" << plg << "is not a conflip plugin";
			} else
				qWarning() << "Failed to load plugin" << plg << "with error" << loader->errorString();
		} else
			qWarning() << "File" << plg << "is not a conflip plugin";

		loader->deleteLater();
	}
}

void PluginLoader::loadPlugins()
{
	pluginLoader();
}

QMap<QString, QString> PluginLoader::typeNames()
{
	QMap<QString, QString> res;
	for(auto it = pluginLoader->_plugins.constBegin(); it != pluginLoader->_plugins.constEnd(); it++)
		res.insert(it.key(), it.value()->displayName(it.key()));
	return res;
}

QMap<QString, QStringList> PluginLoader::fileFilters()
{
	QMap<QString, QStringList> res;
	for(auto it = pluginLoader->_plugins.constBegin(); it != pluginLoader->_plugins.constEnd(); it++)
		res.insert(it.key(), it.value()->fileFilters(it.key()));
	return res;
}

SettingsFile *PluginLoader::createSettings(const QString &path, const QString &type, QObject *parent)
{
	auto plugin = pluginLoader->_plugins.value(type);
	if(plugin)
		return plugin->createSettings(path, type, parent);
	else
		throw SettingsLoadException("No plugin found for type: " + type.toUtf8());
}
