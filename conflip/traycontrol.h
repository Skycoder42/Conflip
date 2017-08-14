#ifndef TRAYCONTROL_H
#define TRAYCONTROL_H

#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>
#include <QScopedPointer>
#include <QtDataSync/SyncController>
#include "managesettingsdialog.h"
#include "changeremotedialog.h"

class TrayControl : public QObject
{
	Q_OBJECT

public:
	explicit TrayControl(QObject *parent = nullptr);

private slots:
	void trayAction(QSystemTrayIcon::ActivationReason reason);

	void manageSync();
	void importId();
	void exportId();
	void editRemote();
	void about();

	void syncStateChanged(QtDataSync::SyncController::SyncState syncState);
	void authError(const QString &error);

private:
	QSystemTrayIcon *_tray;
	QScopedPointer<QMenu> _trayMenu;
	QtDataSync::SyncController *_controller;

	QPointer<ManageSettingsDialog> _dialog;
	QPointer<ChangeRemoteDialog> _remoteDialog;

	template <typename T>
	void tryOpen(QPointer<T> &member);
};

#endif // TRAYCONTROL_H
