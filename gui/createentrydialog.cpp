#include "createentrydialog.h"
#include "ui_createentrydialog.h"
#include <dialogmaster.h>
#include <conflip.h>
#include <settings.h>

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

	if(Settings::instance()->gui.createentrydialog.size.isSet())
		resize(Settings::instance()->gui.createentrydialog.size);
}

CreateEntryDialog::~CreateEntryDialog()
{
	Settings::instance()->gui.createentrydialog.size = size();

	delete ui;
}

SyncEntry CreateEntryDialog::createEntry(QWidget *parent)
{
	return editEntry({}, parent);
}

SyncEntry CreateEntryDialog::editEntry(const SyncEntry &entry, QWidget *parent)
{
	CreateEntryDialog dialog(parent);

	if(entry) {
		dialog.ui->pathLineEdit->setText(entry.pathPattern);
		dialog.ui->modeComboBox->setCurrentText(entry.mode);
		dialog.ui->hiddenFilesCheckBox->setChecked(entry.includeHidden);
		dialog.ui->caseSensitiveCheckBox->setChecked(entry.caseSensitive);
		for(const auto& extra : entry.extras) {
			auto item = new QListWidgetItem(extra, dialog.ui->listWidget);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
		}
	}

	if(dialog.exec() == QDialog::Accepted) {
		SyncEntry resEntry;
		resEntry.setCleanPathPattern(dialog.ui->pathLineEdit->text());
		resEntry.mode = dialog.ui->modeComboBox->currentText();
		resEntry.includeHidden = dialog.ui->hiddenFilesCheckBox->isChecked();
		resEntry.caseSensitive = dialog.ui->caseSensitiveCheckBox->isChecked();
		for(auto i = 0; i < dialog.ui->listWidget->count(); i++)
			resEntry.extras.append(dialog.ui->listWidget->item(i)->text());
		return resEntry;
	} else
		return {};
}

void CreateEntryDialog::on_actionSelect_File_triggered()
{
	auto path = DialogMaster::getOpenFileName(this, tr("Select a file"));
	if(!path.isEmpty())
		ui->pathLineEdit->setText(path);
}

void CreateEntryDialog::on_actionSelect_Directory_triggered()
{
	auto path = DialogMaster::getExistingDirectory(this, tr("Select a directory"));
	if(!path.isEmpty())
		ui->pathLineEdit->setText(path);
}

void CreateEntryDialog::on_action_Add_Extra_triggered()
{
	auto item = new QListWidgetItem(ui->listWidget);
	item->setFlags(item->flags() | Qt::ItemIsEditable);
	ui->listWidget->setCurrentItem(item);
	ui->listWidget->editItem(item);
}

void CreateEntryDialog::on_action_Remove_Extra_triggered()
{
	auto item = ui->listWidget->currentItem();
	if(item)
		delete item;
}
