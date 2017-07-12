#include <QApplication>
#include <QJsonSerializer>
#include <QtDataSync/Setup>
#include "libconflip_global.h"
#include "pluginloader.h"
#include "settingsdatabase.h"

#include "editsettingsobjectdialog.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QApplication::setApplicationName(QStringLiteral(TARGET));
	QApplication::setApplicationVersion(QStringLiteral(VERSION));
	QApplication::setOrganizationName(QStringLiteral(COMPANY));
	QApplication::setOrganizationDomain(QStringLiteral(BUNDLE));
	QApplication::setApplicationDisplayName(QStringLiteral(DISPLAY_NAME));

	// register types
	QJsonSerializer::registerAllConverters<SettingsObject>();
	QJsonSerializer::registerAllConverters<SettingsEntry>();
	QJsonSerializer::registerAllConverters<SettingsValue>();

	// load plugins
	PluginLoader::loadPlugins();

	//setup datasync
	QtDataSync::Setup()
			.setDataMerger(new SettingsObjectMerger())
			.create();

	EditSettingsObjectDialog::createObject();

	return 0;
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
