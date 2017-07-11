#include <QApplication>
#include <QJsonSerializer>
#include <QtDataSync/Setup>
#include "libconflip_global.h"
#include "pluginloader.h"
#include "settingsdatabase.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// register types
	QJsonSerializer::registerAllConverters<SettingsEntry>();
	QJsonSerializer::registerAllConverters<SettingsValue>();
	QJsonSerializer::registerAllConverters<SettingsObject>();

	// load plugins
	PluginLoader::loadPlugins();

	//setup datasync
	QtDataSync::Setup()
			.setDataMerger(new SettingsObjectMerger())
			.create();

	return 0;
	return a.exec();
}

QUuid deviceId()
{
	QSettings settings;
	auto id = settings.value(QStringLiteral("device-id")).toUuid();
	if(id.isNull()) {
		id = QUuid::createUuid();
		settings.setValue(QStringLiteral("device-id"), id);
	}
	return id;
}
