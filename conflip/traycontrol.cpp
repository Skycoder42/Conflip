#include "pluginloader.h"
#include "synclogger.h"
#include "traycontrol.h"

#include <QApplication>
#include <QStandardPaths>
#include <QtDataSync/Setup>
#include <dialogmaster.h>

TrayControl::TrayControl(QObject *parent) :
	QObject(parent),
	_tray(new QSystemTrayIcon(QIcon(QStringLiteral(":/icons/tray/main.ico")), this)),
	_trayMenu(new QMenu()),
	_controller(new QtDataSync::SyncController(this)),
	_dialog(nullptr)
{
	_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("configure")),
						 tr("Manage Synchronization"),
						 this, &TrayControl::manageSync);
	_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("text-x-log")),
						 tr("Open Logfile"),
						 this, [](){
		SyncLogger::openLogfile();
	});

	auto menu = _trayMenu->addMenu(QIcon::fromTheme(QStringLiteral("view-refresh")),
								   tr("Synchronization"));
	menu->addAction(QIcon::fromTheme(QStringLiteral("view-refresh")),
					tr("Synchronize"),
					_controller, &QtDataSync::SyncController::triggerSync);
	menu->addSeparator();
	menu->addAction(QIcon::fromTheme(QStringLiteral("document-import")),
					tr("Import Identity"),
					this, &TrayControl::importId);
	menu->addAction(QIcon::fromTheme(QStringLiteral("document-export")),
					tr("Export Identity"),
					this, &TrayControl::exportId);
	menu->addSeparator();
	menu->addAction(tr("Re-Synchronize"),
					_controller, &QtDataSync::SyncController::triggerResync);
	menu->addAction(tr("Change remote"),
					this, &TrayControl::editRemote);
	menu->addAction(tr("Reset Identity"),
					this, &TrayControl::resetId);

	_trayMenu->addSeparator();
	_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("help-about")),
						 tr("About"),
						 this, &TrayControl::about);
	_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("gtk-quit")),
						 tr("Quit"),
						 qApp, &QApplication::quit);

	connect(_tray, &QSystemTrayIcon::activated,
			this, &TrayControl::trayAction);

	connect(_controller, &QtDataSync::SyncController::syncStateChanged,
			this, &TrayControl::syncStateChanged);
	connect(_controller, &QtDataSync::SyncController::authenticationErrorChanged,
			this, &TrayControl::authError);

	_tray->setToolTip(QApplication::applicationDisplayName());
	_tray->setContextMenu(_trayMenu.data());
	_tray->show();
}

void TrayControl::trayAction(QSystemTrayIcon::ActivationReason reason)
{
		switch (reason) {
		case QSystemTrayIcon::Trigger:
			manageSync();
			break;
		default:
			break;
		}
}

void TrayControl::manageSync()
{
	tryOpen(_dialog);
}

void TrayControl::importId()
{
	auto path = DialogMaster::getOpenFileName(nullptr,
											  tr("Import Identity"),
											  QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
											  tr("Datasync Identity (*.dsi);;All Files (*)"));
	if(!path.isNull()) {
		auto file = new QFile(path);
		if(!file->open(QIODevice::ReadOnly)) {
			qCritical() << "Failed to open file for import with error:" << file->errorString();
			DialogMaster::critical(nullptr, tr("Failed to open file!"), tr("Import Identity"));
			return;
		}

		auto authenticator = QtDataSync::Setup::authenticatorForSetup<QtDataSync::WsAuthenticator>(this);
		auto task = authenticator->importUserData(file);
		task.onResult([authenticator, file](){
			DialogMaster::information(nullptr, tr("Identity successfully loaded."), tr("Import Identity"));
			authenticator->deleteLater();
			file->close();
			file->deleteLater();
		}, [authenticator, file](const QException &e) {
			qCritical() << "Identity import failed with error:" << e.what();
			DialogMaster::critical(nullptr, tr("Failed to import identity. Make shure the file is a valid dsi file."), tr("Import Identity"));
			authenticator->deleteLater();
			file->close();
			file->deleteLater();
		});
	}
}

void TrayControl::exportId()
{
	auto path = DialogMaster::getSaveFileName(nullptr,
											  tr("Export Identity"),
											  QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
											  tr("Datasync Identity (*.dsi);;All Files (*)"));
	if(!path.isNull()) {
		QFile file(path);
		if(!file.open(QIODevice::WriteOnly)) {
			qCritical() << "Failed to open file for import with error:" << file.errorString();
			DialogMaster::critical(nullptr, tr("Failed to open file!"), tr("Export Identity"));
			return;
		}

		auto authenticator = QtDataSync::Setup::authenticatorForSetup<QtDataSync::WsAuthenticator>(this);
		authenticator->exportUserData(&file);
		authenticator->deleteLater();
		file.close();
	}
}

void TrayControl::editRemote()
{
	tryOpen(_remoteDialog);
}

void TrayControl::resetId()
{
	if(DialogMaster::question(nullptr,
							  tr("Do you really want to reset your identity? "
								 "This will delete all your synchronized data "
								 "(But not your actual settings files)"))
	   == QMessageBox::No)
		return;

	auto authenticator = QtDataSync::Setup::authenticatorForSetup<QtDataSync::WsAuthenticator>(this);
	auto task = authenticator->resetUserIdentity();
	task.onResult([authenticator](){
		DialogMaster::information(nullptr, tr("Identity successfully resetted."), tr("Reset Identity"));
		authenticator->deleteLater();
	}, [authenticator](const QException &e) {
		qCritical() << "Identity reset failed with error:" << e.what();
		DialogMaster::critical(nullptr, tr("Failed to reset identity."), tr("Reset Identity"));
		authenticator->deleteLater();
	});
}

void TrayControl::about()
{
	auto info = DialogMaster::createInformation();
	info.icon = QApplication::windowIcon();
	info.windowTitle = tr("About");
	info.title = tr("%1 — Version %2")
				 .arg(QApplication::applicationDisplayName())
				 .arg(QApplication::applicationVersion());
	info.text = tr("<p>A tool to synchronize settings/configurations across multiple machines.</p>"
				   "<p>Loaded Plugins: <i>%1</i><br/>"
				   "Qt-Version: <a href=\"https://www.qt.io/\">%2</a></p>"
				   "<p>Developed by: Skycoder42<br/>"
				   "Project Website: <a href=\"https://github.com/Skycoder42/Conflip\">https://github.com/Skycoder42/Conflip</a><br/>"
				   "License: <a href=\"https://github.com/Skycoder42/conflip/blob/master/LICENSE\">BSD 3 Clause</a></p>"
				   "<p>Icons based on <a href=\"http://www.flaticon.com/authors/roundicons\">Roundicons</a> and "
				   "<a href=\"http://www.flaticon.com/authors/creaticca-creative-agency\">Creaticca Creative Agency</a> "
				   "from <a href=\"http://www.flaticon.com\">www.flaticon.com</a> and licensed by <a href=\"http://creativecommons.org/licenses/by/3.0/\">CC 3.0 BY</a></p>")
				.arg(PluginLoader::typeNames().keys().join(tr(", ")))
				.arg(QStringLiteral(QT_VERSION_STR));

	info.buttons = QMessageBox::Close | QMessageBox::Help;
	info.buttonTexts.insert(QMessageBox::Help, tr("About Qt"));
	info.defaultButton = QMessageBox::Close;
	info.escapeButton = QMessageBox::Close;

	auto btn = DialogMaster::messageBox(info);
	if(btn == QMessageBox::Help)
		QApplication::aboutQt();
}

void TrayControl::syncStateChanged(QtDataSync::SyncController::SyncState syncState)
{
	switch (syncState) {
		break;
	case QtDataSync::SyncController::Disconnected:
		_tray->setIcon(QIcon(QStringLiteral(":/icons/tray/disconnected.ico")));
		_tray->setToolTip(tr("%1: Server not connected")
						  .arg(QApplication::applicationDisplayName()));
		break;
	case QtDataSync::SyncController::Loading:
	case QtDataSync::SyncController::Syncing:
		_tray->setIcon(QIcon(QStringLiteral(":/icons/tray/syncing.ico")));
		_tray->setToolTip(tr("%1: Synchronizing changes…")
						  .arg(QApplication::applicationDisplayName()));
		break;
	case QtDataSync::SyncController::Synced:
		_tray->setIcon(QIcon(QStringLiteral(":/icons/tray/main.ico")));
		_tray->setToolTip(tr("%1: Synchronized")
						  .arg(QApplication::applicationDisplayName()));
		break;
	case QtDataSync::SyncController::SyncedWithErrors:
		_tray->setIcon(QIcon(QStringLiteral(":/icons/tray/error.ico")));
		_tray->setToolTip(tr("%1: Synchronization Error! Check the log")
						  .arg(QApplication::applicationDisplayName()));
		break;
	default:
		Q_UNREACHABLE();
	}
}

void TrayControl::authError(const QString &error)
{
	if(error.isNull())
		return;

	qCritical() << "Authentication failed with error:" << error;
	DialogMaster::critical(nullptr,
						   error,
						   tr("Authentication Failed!"));
}

template<typename T>
void TrayControl::tryOpen(QPointer<T> &member)
{
	if(member) {
		if(member->isMinimized())
			member->showNormal();
		else
			member->show();
		member->raise();
		member->activateWindow();
	} else {
		member = new T();
		member->setAttribute(Qt::WA_DeleteOnClose);
		member->open();
	}
}
