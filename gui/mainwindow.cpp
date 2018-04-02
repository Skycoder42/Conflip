#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QProcess>
#include <conflip.h>
#include <dialogmaster.h>
#include <createentrydialog.h>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	connect(ui->action_Exit, &QAction::triggered,
			this, &QMainWindow::close);
	connect(ui->actionAbout_Qt, &QAction::triggered,
			qApp, &QApplication::aboutQt);

	auto sp = new QAction(this);
	sp->setSeparator(true);
	ui->treeView->addActions({
								 ui->action_Add_Entry,
								 ui->action_Edit_Entry,
								 sp,
								 ui->action_Remove_Entry
							 });
}

MainWindow::~MainWindow()
{
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
	CreateEntryDialog::createEntry(this);
}

void MainWindow::on_action_Edit_Entry_triggered()
{

}

void MainWindow::on_action_Remove_Entry_triggered()
{

}

void MainWindow::on_action_About_triggered()
{
	auto info = DialogMaster::createInformation();
	info.icon = QApplication::windowIcon();
	info.windowTitle = tr("About");
	info.title = tr("%1 — Version %2")
				 .arg(QApplication::applicationDisplayName())
				 .arg(QApplication::applicationVersion());
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
				.arg(Conflip::listPlugins().join(QStringLiteral(", ")))
				.arg(QStringLiteral(QT_VERSION_STR));

	info.buttons = QMessageBox::Close;
	info.defaultButton = QMessageBox::Close;
	info.escapeButton = QMessageBox::Close;
	DialogMaster::messageBox(info);
}
