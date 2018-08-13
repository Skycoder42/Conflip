#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QProcess>
#include <QClipboard>
#include <conflip.h>
#include <conflipdatabase.h>
#include <dialogmaster.h>
#include <createentrydialog.h>
#include <settings.h>
#include <QSaveFile>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
#ifdef QT_NO_DEBUG
	_svcControl{QtService::ServiceControl::create(QStringLiteral(SERVICE_BACKEND),
												  QCoreApplication::applicationName(), //includes the slice on linux
												  this)},
#else
	_svcControl{QtService::ServiceControl::create(QStringLiteral("standard"),
												  QDir{QCoreApplication::applicationDirPath()}.absoluteFilePath(QStringLiteral(TARGET "@test")),
												  this)},
#endif
	_model(new QGadgetListModel<SyncEntry>(this)),
	_proxyModel(new QObjectProxyModel({tr("Mode"), tr("Path")}, this)),
	_sortModel(new QSortFilterProxyModel(this)),
	_serializer(new QJsonSerializer(this)),
	_watcher(new QFileSystemWatcher(this)),
	_rmList()
{
	ui->setupUi(this);

	// setup daemon connection
	if(!_svcControl->serviceExists()) {
		ui->action_Reload_Daemon->setVisible(false);
		DialogMaster::warning(nullptr,
							  tr("Failed to find the %1 service by service id: %2")
							  .arg(QApplication::applicationDisplayName(), _svcControl->serviceId()),
							  tr("Service not found!"));
	}

	// prepare treeview and it's models
	auto sp = new QAction(this);
	sp->setSeparator(true);
	ui->treeView->addActions({
								 ui->action_Add_Entry,
								 ui->action_Edit_Entry,
								 sp,
								 ui->action_Remove_Entry
							 });

	_proxyModel->setSourceModel(_model);
	_proxyModel->addMapping(0, Qt::DisplayRole, "mode");
	_proxyModel->addMapping(1, Qt::DisplayRole, "pathPattern");

	_sortModel->setSourceModel(_proxyModel);
	_sortModel->setSortLocaleAware(true);

	ui->treeView->setModel(_sortModel);
	ui->treeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui->treeView->header()->setSectionResizeMode(1, QHeaderView::Stretch);

	// prepare watcher
	if(Settings::instance()->engine.dir.isSet()) {
		QString path = Settings::instance()->engine.dir;
		_watcher->addPath(path + QStringLiteral("/config.json"));
		ui->pathedit->setPath(path);
		QMetaObject::invokeMethod(this, "reload", Qt::QueuedConnection);
	}

	//connections
	connect(ui->action_Exit, &QAction::triggered,
			this, &QMainWindow::close);
	connect(ui->actionAbout_Qt, &QAction::triggered,
			qApp, &QApplication::aboutQt);
	connect(ui->treeView, &QTreeView::activated,
			ui->action_Edit_Entry, &QAction::trigger);
	connect(ui->pathedit, &QPathEdit::pathChanged,
			this, &MainWindow::updatePath);
	connect(_watcher, &QFileSystemWatcher::fileChanged,
			this, &MainWindow::reload);

	// restore geom
	if(Settings::instance()->gui.mainwindow.geom.isSet())
		restoreGeometry(Settings::instance()->gui.mainwindow.geom);
	if(Settings::instance()->gui.mainwindow.state.isSet())
		restoreState(Settings::instance()->gui.mainwindow.state);
	if(Settings::instance()->gui.mainwindow.header.isSet())
		ui->treeView->header()->restoreState(Settings::instance()->gui.mainwindow.header);
}

MainWindow::~MainWindow()
{
	Settings::instance()->gui.mainwindow.geom = saveGeometry();
	Settings::instance()->gui.mainwindow.state = saveState();
	Settings::instance()->gui.mainwindow.header = ui->treeView->header()->saveState();

#ifndef QT_NO_DEBUG
	if(_svcControl->serviceExists())
		_svcControl->stop(); //dont care for result
#endif

	delete ui;
}

void MainWindow::on_action_Reload_Daemon_triggered()
{
	if(_svcControl->serviceExists()) {
		if(!_svcControl->reload()) {
			DialogMaster::critical(this,
								   tr("Failed to reload %1 service with error: %2")
								   .arg(QApplication::applicationDisplayName(), _svcControl->error()),
								   tr("Service Error"));
		}
	}
}

void MainWindow::on_action_Add_Entry_triggered()
{
	auto entry = CreateEntryDialog::createEntry(this);
	if(entry) {
		_model->addGadget(entry);
		update();
	}
}

void MainWindow::on_action_Edit_Entry_triggered()
{
	auto index = ui->treeView->currentIndex();
	index = _sortModel->mapToSource(index);
	index = _proxyModel->mapToSource(index);
	if(index.isValid()) {
		auto entry = _model->gadget(index);
		entry = CreateEntryDialog::editEntry(entry, this);
		if(entry) {
			_model->removeGadget(index);
			_model->insertGadget(index, entry);
			update();
		}
	}
}

void MainWindow::on_action_Remove_Entry_triggered()
{
	auto index = ui->treeView->currentIndex();
	index = _sortModel->mapToSource(index);
	index = _proxyModel->mapToSource(index);
	if(index.isValid()) {
		auto entry = _model->takeGadget(index);
		if(!entry.syncedMachines.isEmpty())
			_rmList.append(entry);
		update();
	}
}

void MainWindow::on_action_About_triggered()
{
	auto info = DialogMaster::createInformation();
	info.icon = QApplication::windowIcon();
	info.windowTitle = tr("About");
	info.title = tr("%1 — Version %2")
				 .arg(QApplication::applicationDisplayName(), QApplication::applicationVersion());
	info.text = tr("<p>A tool to synchronize settings/configurations across multiple machines.</p>"
				   "<p>Available Plugins: <i>%1</i><br/>"
				   "Compiled with Qt-Version: <a href=\"https://www.qt.io/\">%2</a></p>"
				   "<p>Developed by: Skycoder42<br/>"
				   "Project Website: <a href=\"https://github.com/Skycoder42/Conflip\">https://github.com/Skycoder42/Conflip</a><br/>"
				   "License: <a href=\"https://github.com/Skycoder42/Conflip/blob/master/LICENSE\">BSD 3 Clause</a></p>"
				   "<p>Icons based on <a href=\"http://www.flaticon.com/authors/roundicons\">Roundicons</a> "
				   "and <a href=\"http://www.flaticon.com/authors/creaticca-creative-agency\">Creaticca Creative Agency</a> "
				   "from <a href=\"http://www.flaticon.com\">www.flaticon.com</a> "
				   "and are licensed by <a href=\"http://creativecommons.org/licenses/by/3.0/\">CC 3.0 BY</a></p>")
				.arg(Conflip::listPlugins().join(QStringLiteral(", ")), QStringLiteral(QT_VERSION_STR));

	info.buttons = QMessageBox::Close;
	info.defaultButton = QMessageBox::Close;
	info.escapeButton = QMessageBox::Close;
	DialogMaster::messageBox(info);
}

void MainWindow::on_actionPaste_Path_triggered()
{
	auto clipBoard = QApplication::clipboard();
	auto path = clipBoard->text();
	QUrl pathUrl{path};
	if(pathUrl.isLocalFile())
		path = pathUrl.toLocalFile();
	if(path.isEmpty())
		return;
	auto entry = CreateEntryDialog::editEntry(SyncEntry{path}, this);
	if(entry) {
		_model->addGadget(entry);
		update();
	}
}

void MainWindow::reload()
{
	ui->action_Add_Entry->setEnabled(false);
	ui->action_Edit_Entry->setEnabled(false);
	ui->action_Remove_Entry->setEnabled(false);
	ui->action_Reload_Daemon->setEnabled(false);

	try {
		_model->resetModel({});
		_rmList.clear();

		QDir pathDir{Settings::instance()->engine.dir};
		QFile file{pathDir.absoluteFilePath(Conflip::ConfigFileName())};
		if(!file.exists())
			return;

		if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			DialogMaster::critical(this, file.errorString(), tr("Reading config failed"));
			return;
		}

		auto db = _serializer->deserializeFrom<ConflipDatabase>(&file);
		_model->resetModel(db.entries);
		file.close();

		ui->action_Add_Entry->setEnabled(true);
		ui->action_Edit_Entry->setEnabled(true);
		ui->action_Remove_Entry->setEnabled(true);
		ui->action_Reload_Daemon->setEnabled(true);

		if(_svcControl->serviceExists()) {
			switch(_svcControl->status()) {
			case QtService::ServiceControl::ServiceStopped:
			case QtService::ServiceControl::ServiceErrored:
				if(!_svcControl->start()) {
					DialogMaster::critical(this,
										   tr("Failed to start %1 service with error: %2")
										   .arg(QApplication::applicationDisplayName(), _svcControl->error()),
										   tr("Service Error"));
				}
				break;
			case QtService::ServiceControl::ServicePaused:
				if(!_svcControl->resume()) {
					DialogMaster::critical(this,
										   tr("Failed to resume %1 service with error: %2")
										   .arg(QApplication::applicationDisplayName(), _svcControl->error()),
										   tr("Service Error"));
					break;
				}
				Q_FALLTHROUGH();
			case QtService::ServiceControl::ServiceRunning:
				on_action_Reload_Daemon_triggered();
				break;
			case QtService::ServiceControl::ServiceStatusUnknown:
				if(!_svcControl->start())
					on_action_Reload_Daemon_triggered();
				break;
			case QtService::ServiceControl::ServiceStarting:
			case QtService::ServiceControl::ServiceResuming:
			case QtService::ServiceControl::ServiceReloading:
			case QtService::ServiceControl::ServiceStopping:
			case QtService::ServiceControl::ServicePausing:
				break;
			default:
				Q_UNREACHABLE();
				break;
			}
		}

		auto hasError = db.hasErrors.value(Settings::instance()->engine.machineid.get().toString(QUuid::WithoutBraces), false);
		if(hasError) {
			DialogMaster::critical(this,
								   tr("An error occured in the last synchronization. Check the system error log for details."),
								   tr("Synchronization Error"));
		}
	} catch(QJsonSerializationException &e) {
		qCritical() << e.what();
		DialogMaster::critical(this, tr("Parsed JSON content is invalid!"), tr("Reading config failed"));
	}
}

void MainWindow::update()
{
	try {
		QDir pathDir{Settings::instance()->engine.dir};
		ConflipDatabase db;

		QFile file(pathDir.absoluteFilePath(Conflip::ConfigFileName()));
		if(file.exists()) {
			if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
				DialogMaster::critical(this, file.errorString(), tr("Reading config failed"));
				return;
			}

			db = _serializer->deserializeFrom<ConflipDatabase>(&file);
			file.close();
		}

		db.entries = _model->gadgets();
		db.unsynced.append(_rmList);

		QSaveFile saveFile(pathDir.absoluteFilePath(Conflip::ConfigFileName()));
		if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			DialogMaster::critical(this, file.errorString(), tr("Writing config failed"));
			return;
		}
		_serializer->serializeTo(&saveFile, db);
		if(!saveFile.commit()) {
			DialogMaster::critical(this, file.errorString(), tr("Writing config failed"));
			return;
		}

		_rmList.clear();
	} catch(QJsonSerializationException &e) {
		qCritical() << e.what();
		DialogMaster::critical(this, tr("Parsed JSON content is invalid!"), tr("Reading config failed"));
	}
}

void MainWindow::updatePath(const QString &path)
{
	QDir pathDir(QDir::cleanPath(path));
	pathDir.makeAbsolute();
	if(pathDir.exists()) {
		Settings::instance()->engine.dir = pathDir.absolutePath();
		if(Conflip::initConfDir()) {
			_watcher->addPath(pathDir.absoluteFilePath(Conflip::ConfigFileName()));
			reload();
		}
	}
}
