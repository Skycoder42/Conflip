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

#include "../../__private/conflip-datasync-secret.h"

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

	//connect to server
	auto auth = QtDataSync::Setup::authenticatorForSetup<QtDataSync::WsAuthenticator>(qApp);
	if(!auth->remoteUrl().isValid()) {
		auth->setRemoteUrl(QStringLiteral("wss://apps.skycoder42.de/conflip/"));
		auth->setServerSecret(QString::fromUtf8(DATASYNC_SERVER_SECRET));
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
