#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QProcess>
#include <conflip.h>
#include <conflipdatabase.h>
#include <dialogmaster.h>
#include <createentrydialog.h>
#include <settings.h>
#include <QSaveFile>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	_model(new QGadgetListModel<SyncEntry>(this)),
	_proxyModel(new QObjectProxyModel({tr("Mode"), tr("Path")}, this)),
	_sortModel(new QSortFilterProxyModel(this)),
	_serializer(new QJsonSerializer(this)),
	_watcher(new QFileSystemWatcher(this)),
	_rmList()
{
	ui->setupUi(this);

	//connections
	connect(ui->action_Exit, &QAction::triggered,
			this, &QMainWindow::close);
	connect(ui->actionAbout_Qt, &QAction::triggered,
			qApp, &QApplication::aboutQt);
	connect(ui->pathedit, &QPathEdit::pathChanged,
			this, &MainWindow::updatePath);
	connect(ui->treeView, &QTreeView::activated,
			ui->action_Edit_Entry, &QAction::trigger);
	connect(_watcher, &QFileSystemWatcher::fileChanged,
			this, &MainWindow::reload);

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
		reload();
	}

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

	delete ui;
}

void MainWindow::on_action_Reload_Daemon_triggered()
{
	auto proc = new QProcess(this);
	proc->setProgram(QStringLiteral("systemctl"));
	proc->setArguments({
						   QStringLiteral("--user"),
						   QStringLiteral("reload"),
						   QCoreApplication::applicationName() + QStringLiteral(".service")
					   });
	proc->setProcessChannelMode(QProcess::ForwardedChannels);
	connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			this, [this, proc](int code, QProcess::ExitStatus exitStatus) {
		if(exitStatus == QProcess::NormalExit && code == EXIT_SUCCESS)
			DialogMaster::information(this, tr("Reloading successful!"));
		else
			DialogMaster::warning(this, tr("Check the applications output log for more details"), tr("Reloading failed!"));
		proc->deleteLater();
	});
	proc->start();
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
				   "Compile with Qt-Version: <a href=\"https://www.qt.io/\">%2</a></p>"
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

void MainWindow::reload()
{
	try {
		_model->resetModel({});
		_rmList.clear();

		QDir pathDir(ui->pathedit->path());
		QFile file(pathDir.absoluteFilePath(Conflip::ConfigFileName()));
		if(!file.exists())
			return;

		if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			DialogMaster::critical(this, file.errorString(), tr("Reading config failed"));
			return;
		}

		auto db = _serializer->deserializeFrom<ConflipDatabase>(&file);
		_model->resetModel(db.entries);
		file.close();
	} catch(QJsonSerializationException &e) {
		qCritical() << e.what();
		DialogMaster::critical(this, tr("Parsed JSON content is invalid!"), tr("Reading config failed"));
	}
}

void MainWindow::update()
{
	try {
		QDir pathDir(ui->pathedit->path());
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
		_watcher->addPath(pathDir.absoluteFilePath(Conflip::ConfigFileName()));
		reload();
	}
}
