#include "editsettingsobjectdialog.h"
#include "ui_editsettingsobjectdialog.h"
#include <dialogmaster.h>
#include <QStandardPaths>
#include <QPushButton>
#include "pluginloader.h"

EditSettingsObjectDialog::EditSettingsObjectDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EditSettingsObjectDialog),
	model(nullptr),
	sortModel(new QSortFilterProxyModel(this)),
	store(DataStore::instance()),
	isCreate(true),
	object()
{
	ui->setupUi(this);
	DialogMaster::masterDialog(this);

	ui->settingsTreeView->addActions({
										 ui->action_Expand_all,
										 ui->action_Collapse_all,
									 });

	connect(ui->action_Expand_all, &QAction::triggered,
			ui->settingsTreeView, &QTreeView::expandAll);
	connect(ui->action_Collapse_all, &QAction::triggered,
			ui->settingsTreeView, &QTreeView::collapseAll);

	auto types = PluginLoader::typeNames();
	for(auto it = types.constBegin(); it != types.constEnd(); it++)
		ui->settingsTypeComboBox->insertItem(0, it.value(), it.key());
	ui->settingsTypeComboBox->setCurrentIndex(0);

	ui->settingsTreeView->setModel(sortModel);
}

EditSettingsObjectDialog::~EditSettingsObjectDialog()
{
	delete ui;
}

void EditSettingsObjectDialog::setup()
{
	if(isCreate)
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	else {
		ui->settingsTypeLabel->setEnabled(false);
		ui->settingsTypeComboBox->setEnabled(false);
		ui->pathIDLineEdit->setText(object.devicePath());
		ui->syncAllBox->setChecked(object.syncAll);
		on_applyButton_clicked();
	}
}

SettingsObject EditSettingsObjectDialog::createObject(QWidget *parent)
{
	EditSettingsObjectDialog dialog(parent);
	dialog.isCreate = true;
	dialog.setup();
	if(dialog.exec() == QDialog::Accepted)
		return dialog.object;
	else
		return SettingsObject();
}

SettingsObject EditSettingsObjectDialog::editObject(SettingsObject object, QWidget *parent)
{
	EditSettingsObjectDialog dialog(parent);
	dialog.isCreate = false;
	dialog.object = object;
	dialog.setup();
	if(dialog.exec() == QDialog::Accepted)
		return dialog.object;
	else
		return SettingsObject();
}

void EditSettingsObjectDialog::accept()
{
	QList<QPair<QStringList, bool>> entries;
	auto all = ui->syncAllBox->isChecked();
	if(!all)
		entries = model->extractEntries();

	if(isCreate) {
		object = store->createObject(ui->settingsTypeComboBox->currentText(),
								  ui->pathIDLineEdit->text(),
								  entries,
								  all);
	} else {
		object = store->updateObject(object,
							   ui->pathIDLineEdit->text(),
							   entries,
							   all);
	}

	QDialog::accept();
}

void EditSettingsObjectDialog::on_openButton_clicked()
{
	auto path = DialogMaster::getOpenFileName(this,
											  tr("Open a settings file"),
											  QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation),
											  ui->settingsTypeComboBox->currentText() + QStringLiteral(";;All Files (*)"));
	if(!path.isNull()) {
		ui->pathIDLineEdit->setText(path);
		on_applyButton_clicked();
	}
}

void EditSettingsObjectDialog::on_applyButton_clicked()
{
	if(model)
		model->deleteLater();
	try {
		auto file = PluginLoader::createSettings(ui->pathIDLineEdit->text(), ui->settingsTypeComboBox->currentData().toString(), this);
		model = new SettingsFileModel(file, this);
		model->enablePreview(ui->dataPreviewCheckBox->isChecked());
		sortModel->setSourceModel(model);
		ui->settingsTreeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

		if(!isCreate)
			model->initialize(object);
	} catch(QException &e) {
		qWarning() << "Failed to load" << ui->pathIDLineEdit->text()
				   << "for type" << ui->settingsTypeComboBox->currentData().toString()
				   << "with error" << e.what();
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		DialogMaster::warning(this, tr("Failed to load settings for the given file/ID"));
	}
}

void EditSettingsObjectDialog::on_dataPreviewCheckBox_clicked(bool checked)
{
	if(model)
		model->enablePreview(checked);
}
