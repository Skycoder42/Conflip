#include "createentrydialog.h"
#include "ui_createentrydialog.h"
#include <QWhatsThis>
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
	ui->extrasListWidget->addActions({
								   ui->action_Add_Extra,
								   ui->action_Remove_Extra
							   });
	ui->helpButton->setDefaultAction(ui->actionWhats_this);
	connect(ui->actionWhats_this, &QAction::triggered,
			this, &CreateEntryDialog::showHint);

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
		dialog.ui->matchDirectoriesCheckBox->setChecked(entry.matchDirs);
		for(const auto& extra : entry.extras) {
			auto item = new QListWidgetItem(extra, dialog.ui->extrasListWidget);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
		}
	}

	if(dialog.exec() == QDialog::Accepted) {
		SyncEntry resEntry;
		resEntry.setCleanPathPattern(dialog.ui->pathLineEdit->text());
		resEntry.mode = dialog.ui->modeComboBox->currentText();
		resEntry.includeHidden = dialog.ui->hiddenFilesCheckBox->isChecked();
		resEntry.caseSensitive =
		dialog.ui->caseSensitiveCheckBox->isChecked();
		resEntry.matchDirs = dialog.ui->matchDirectoriesCheckBox->isEnabled() &&
							 dialog.ui->matchDirectoriesCheckBox->isChecked();
		for(auto i = 0; i < dialog.ui->extrasListWidget->count(); i++)
			resEntry.extras.append(dialog.ui->extrasListWidget->item(i)->text());
		return resEntry;
	} else
		return {};
}

void CreateEntryDialog::showHint()
{
	QWhatsThis::showText(mapToGlobal({width()/2, ui->extrasListWidget->y()}),
						 ui->extrasListWidget->whatsThis(),
						 this);
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
	auto item = new QListWidgetItem(ui->extrasListWidget);
	item->setFlags(item->flags() | Qt::ItemIsEditable);
	ui->extrasListWidget->setCurrentItem(item);
	ui->extrasListWidget->editItem(item);
}

void CreateEntryDialog::on_action_Remove_Extra_triggered()
{
	auto item = ui->extrasListWidget->currentItem();
	if(item)
		delete item;
}

void CreateEntryDialog::on_modeComboBox_currentIndexChanged(const QString &text)
{
	Conflip::loadTranslations(text);
	auto helper = Conflip::loadHelper(text, this);
	if(helper) {
		ui->matchDirectoriesCheckBox->setEnabled(helper->canSyncDirs(text));
		auto info = helper->extrasHint();
		ui->extrasLabel->setEnabled(info.enabled);
		ui->extrasListWidget->setEnabled(info.enabled);
		ui->action_Add_Extra->setEnabled(info.enabled);
		ui->action_Remove_Extra->setEnabled(info.enabled);
		ui->actionWhats_this->setEnabled(info.enabled);
		ui->extrasLabel->setText(info.title + tr(":"));
		ui->extrasLabel->setWhatsThis(info.hint);
		ui->extrasListWidget->setWhatsThis(info.hint);
		helper->deleteLater();
	} else {
		ui->matchDirectoriesCheckBox->setEnabled(true);
		ui->extrasLabel->setEnabled(false);
		ui->extrasListWidget->setEnabled(false);
		ui->action_Add_Extra->setEnabled(false);
		ui->action_Remove_Extra->setEnabled(false);
		ui->actionWhats_this->setEnabled(false);
	}
}
