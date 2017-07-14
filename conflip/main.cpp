#include <QApplication>
#include <QIcon>
#include <QJsonSerializer>
#include <QSqlDatabase>
#include <QtDataSync/Setup>
#include <QtDataSync/SyncController>
#include <QtDataSync/WsAuthenticator>
#include "libconflip_global.h"
#include "pluginloader.h"
#include "settingsdatabase.h"

#include "editsettingsobjectdialog.h"
#include "traycontrol.h"
#include "syncmanager.h"
#include "synclogger.h"

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

	//enable file logging
	SyncLogger::setup(a.arguments().contains(QStringLiteral("--verbose")));

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
	//DEBUG
	QtDataSync::SyncController controller;
	QObject::connect(&controller, &QtDataSync::SyncController::syncStateChanged, [](QtDataSync::SyncController::SyncState state){
		qDebug() << state;
	});

	//connect to server
	auto auth = QtDataSync::Setup::authenticatorForSetup<QtDataSync::WsAuthenticator>(qApp);
	if(!auth->remoteUrl().isValid()) {
		auth->setServerSecret(QStringLiteral("baum42"));
		auth->setRemoteUrl(QStringLiteral("ws://localhost:8080"));
	} else
		delete auth;

	SyncManager manager;
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
