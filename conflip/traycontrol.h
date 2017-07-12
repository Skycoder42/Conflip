#ifndef TRAYCONTROL_H
#define TRAYCONTROL_H

#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>
#include <QScopedPointer>

class TrayControl : public QObject
{
	Q_OBJECT

public:
	explicit TrayControl(QObject *parent = nullptr);

private slots:
	void trayAction(QSystemTrayIcon::ActivationReason reason);

	void manageSync();

private:
	QSystemTrayIcon *_tray;
	QScopedPointer<QMenu> _trayMenu;
};

#endif // TRAYCONTROL_H
