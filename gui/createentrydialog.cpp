#include "createentrydialog.h"
#include "ui_createentrydialog.h"
#include <dialogmaster.h>
#include <conflip.h>

CreateEntryDialog::CreateEntryDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::CreateEntryDialog)
{
	ui->setupUi(this);
	DialogMaster::masterDialog(this);

	ui->selectButton->addActions({
									 ui->actionSelect_File,
									 ui->actionSelect_Directory
								 });
	ui->selectButton->setDefaultAction(ui->actionSelect_File);

	ui->modeComboBox->addItems(Conflip::listPlugins());
	ui->modeComboBox->setCurrentText(QStringLiteral("symlink"));

	ui->addButton->setDefaultAction(ui->action_Add_Extra);
	ui->removeButton->setDefaultAction(ui->action_Remove_Extra);
	ui->listWidget->addActions({
								   ui->action_Add_Extra,
								   ui->action_Remove_Extra
							   });
}

CreateEntryDialog::~CreateEntryDialog()
{
	delete ui;
}

SyncEntry CreateEntryDialog::createEntry(QWidget *parent)
{
	return editEntry({}, parent);
}

SyncEntry CreateEntryDialog::editEntry(const SyncEntry &entry, QWidget *parent)
{
	CreateEntryDialog dialog(parent);

	if(!entry.pathPattern.isNull()) {
		dialog.ui->pathLineEdit->setText(entry.pathPattern);
		dialog.ui->modeComboBox->setCurrentText(entry.mode);
		dialog.ui->hiddenFilesCheckBox->setChecked(entry.includeHidden);
		dialog.ui->caseSensitiveCheckBox->setChecked(entry.caseSensitive);
		dialog.ui->listWidget->addItems(entry.extras);
	}

	if(dialog.exec() == QDialog::Accepted)
		return entry;
	else
		return {};
}

void CreateEntryDialog::on_actionSelect_File_triggered()
{

}

void CreateEntryDialog::on_actionSelect_Directory_triggered()
{

}

void CreateEntryDialog::on_action_Add_Extra_triggered()
{

}

void CreateEntryDialog::on_action_Remove_Extra_triggered()
{

}
