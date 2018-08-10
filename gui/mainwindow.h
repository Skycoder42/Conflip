#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonSerializer>
#include <QFileSystemWatcher>
#include <QSortFilterProxyModel>
#include <QtService/ServiceControl>
#include <qgadgetlistmodel.h>
#include <qobjectproxymodel.h>
#include <syncentry.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override;

private slots:
	void on_action_Reload_Daemon_triggered();
	void on_action_Add_Entry_triggered();
	void on_action_Edit_Entry_triggered();
	void on_action_Remove_Entry_triggered();
	void on_action_About_triggered();
	void on_actionPaste_Path_triggered();

	void reload();
	void update();
	void updatePath(const QString &path);

private:
	Ui::MainWindow *ui;
	QtService::ServiceControl *_svcControl;

	QGadgetListModel<SyncEntry> *_model;
	QObjectProxyModel *_proxyModel;
	QSortFilterProxyModel *_sortModel;

	QJsonSerializer *_serializer;
	QFileSystemWatcher *_watcher;

	QList<SyncEntry> _rmList;
};

#endif // MAINWINDOW_H
