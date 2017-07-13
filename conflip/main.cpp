#include <QApplication>
#include <QIcon>
#include <QJsonSerializer>
#include <QSqlDatabase>
#include <QtDataSync/Setup>
#include "libconflip_global.h"
#include "pluginloader.h"
#include "settingsdatabase.h"

#include "editsettingsobjectdialog.h"
#include "traycontrol.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QApplication::setApplicationName(QStringLiteral(TARGET));
	QApplication::setApplicationVersion(QStringLiteral(VERSION));
	QApplication::setOrganizationName(QStringLiteral(COMPANY));
	QApplication::setOrganizationDomain(QStringLiteral(BUNDLE));
	QApplication::setApplicationDisplayName(QStringLiteral(DISPLAY_NAME));
	QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/main.svg")));
	QApplication::setQuitOnLastWindowClosed(false);

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

//	DataStore s;
//	s.loadAll<SettingsObject>().onResult([](QList<SettingsObject> o){
//		if(o.isEmpty())
//			EditSettingsObjectDialog::createObject();
//		else
//			EditSettingsObjectDialog::editObject(o.first());
//	});

	TrayControl tray;
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
