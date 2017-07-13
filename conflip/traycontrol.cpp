#include "pluginloader.h"
#include "synclogger.h"
#include "traycontrol.h"

#include <QApplication>
#include <dialogmaster.h>

TrayControl::TrayControl(QObject *parent) :
	QObject(parent),
	_tray(new QSystemTrayIcon(QIcon(QStringLiteral(":/icons/tray/main.ico")), this)),
	_trayMenu(new QMenu()),
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

	_trayMenu->addSeparator();
	_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("help-about")),
						 tr("About"),
						 this, &TrayControl::about);
	_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("gtk-quit")),
						 tr("Quit"),
						 qApp, &QApplication::quit);

	connect(_tray, &QSystemTrayIcon::activated,
			this, &TrayControl::trayAction);

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
	if(_dialog) {
		if(_dialog->isMinimized())
			_dialog->showNormal();
		else
			_dialog->show();
		_dialog->raise();
		_dialog->activateWindow();
	} else {
		_dialog = new ManageSettingsDialog();
		_dialog->setAttribute(Qt::WA_DeleteOnClose);
		_dialog->open();
	}
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
