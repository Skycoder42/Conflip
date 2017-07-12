#include "traycontrol.h"

#include <QApplication>

TrayControl::TrayControl(QObject *parent) :
	QObject(parent),
	_tray(new QSystemTrayIcon(QIcon(QStringLiteral(":/icons/tray/main.ico")), this)),
	_trayMenu(new QMenu())
{
	_trayMenu->addAction(QIcon::fromTheme(QStringLiteral("gtk-quit")),
						 tr("Quit"),
						 qApp, &QApplication::quit);

	_tray->setToolTip(QApplication::applicationDisplayName());
	_tray->setContextMenu(_trayMenu.data());
	_tray->show();
}
