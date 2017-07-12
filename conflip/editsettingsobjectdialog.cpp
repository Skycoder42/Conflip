#include "editsettingsobjectdialog.h"
#include "ui_editsettingsobjectdialog.h"
#include <dialogmaster.h>
#include <QStandardPaths>
#include "pluginloader.h"

EditSettingsObjectDialog::EditSettingsObjectDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EditSettingsObjectDialog),
	model(nullptr),
	store(new DataStore(this)),
	isCreate(true),
	object()
{
	ui->setupUi(this);
	DialogMaster::masterDialog(this);

	auto types = PluginLoader::typeNames();
	for(auto it = types.constBegin(); it != types.constEnd(); it++)
		ui->settingsTypeComboBox->insertItem(0, it.value(), it.key());
	ui->settingsTypeComboBox->setCurrentIndex(0);
}

EditSettingsObjectDialog::~EditSettingsObjectDialog()
{
	delete ui;
}

SettingsObject EditSettingsObjectDialog::createObject(QWidget *parent)
{
	EditSettingsObjectDialog dialog(parent);
	dialog.isCreate = true;
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
	if(dialog.exec() == QDialog::Accepted)
		return dialog.object;
	else
		return SettingsObject();
}

void EditSettingsObjectDialog::accept()
{
	object = store->createNew(ui->settingsTypeComboBox->currentText(),
							  ui->pathIDLineEdit->text(),
							  {});
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
		ui->settingsTreeView->setModel(model);
	} catch(QException &e) {
		qWarning() << "Failed to load" << ui->pathIDLineEdit->text()
				   << "for type" << ui->settingsTypeComboBox->currentData().toString()
				   << "with error" << e.what();
		//TODO message box
	}
}

void EditSettingsObjectDialog::showEvent(QShowEvent *event)
{
	if(isCreate)
		ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel);

	QDialog::showEvent(event);
}
