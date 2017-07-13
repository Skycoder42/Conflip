#include "traycontrol.h"

#include <QApplication>

TrayControl::TrayControl(QObject *parent) :
	QObject(parent),
	_tray(new QSystemTrayIcon(QIcon(QStringLiteral(":/icons/tray/main.ico")), this)),
	_trayMenu(new QMenu()),
	_dialog(nullptr)
{
	_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("configure")),
						 tr("Manage Synchronization"),
						 this, &TrayControl::manageSync);

	_trayMenu->addSeparator();
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
