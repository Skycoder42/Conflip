#include "managesettingsdialog.h"
#include "ui_managesettingsdialog.h"
#include <dialogmaster.h>
#include "editsettingsobjectdialog.h"

ManageSettingsDialog::ManageSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ManageSettingsDialog),
	_store(DataStore::instance())
{
	ui->setupUi(this);
	DialogMaster::masterDialog(this);

	ui->toolButton->setDefaultAction(ui->action_Add_Settings);

	auto sep = new QAction(this);
	sep->setSeparator(true);
	ui->listWidget->addActions({
								   ui->action_Add_Settings,
								   sep,
								   ui->action_Edit,
								   ui->action_Remove
							   });

	_store->loadAll<SettingsObject>().onResult(this, [this](QList<SettingsObject> list){
		foreach(auto obj, list) {
			auto item = new QListWidgetItem(obj.devicePath(), ui->listWidget);
			item->setData(Qt::UserRole, QVariant::fromValue(obj));
		}
	}, [this](const QException &e){
		qWarning() << "Failed to load settings objects with error"
				   << e.what();
		DialogMaster::critical(this, tr("Failed to load synchronized Settings!"));
	});
}

ManageSettingsDialog::~ManageSettingsDialog()
{
	delete ui;
}

void ManageSettingsDialog::on_listWidget_itemActivated(QListWidgetItem *item)
{
	if(item) {
		auto obj = EditSettingsObjectDialog::editObject(item->data(Qt::UserRole).value<SettingsObject>(), this);
		if(obj.isValid()) {
			item->setText(obj.devicePath());
			item->setData(Qt::UserRole, QVariant::fromValue(obj));
		}
	}
}

void ManageSettingsDialog::on_action_Add_Settings_triggered()
{
	auto obj = EditSettingsObjectDialog::createObject(this);
	if(obj.isValid()) {
		auto item = new QListWidgetItem(obj.devicePath(), ui->listWidget);
		item->setData(Qt::UserRole, QVariant::fromValue(obj));
	}
}

void ManageSettingsDialog::on_action_Edit_triggered()
{
	on_listWidget_itemActivated(ui->listWidget->currentItem());
}

void ManageSettingsDialog::on_action_Remove_triggered()
{
	auto item = ui->listWidget->currentItem();
	if(item) {
		item->setFlags(Qt::NoItemFlags);
		_store->removeObject(item->data(Qt::UserRole).value<SettingsObject>());
		delete item;
	}
}
