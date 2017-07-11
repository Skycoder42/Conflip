#include "editsettingsobjectdialog.h"
#include "ui_editsettingsobjectdialog.h"
#include <dialogmaster.h>
#include <QStandardPaths>
#include "pluginloader.h"

EditSettingsObjectDialog::EditSettingsObjectDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EditSettingsObjectDialog)
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
	dialog.ui->buttonBox->setStandardButtons(QDialogButtonBox::Cancel);

	if(dialog.exec() == QDialog::Accepted)
		return SettingsObject();
}

SettingsObject EditSettingsObjectDialog::editObject(SettingsObject object, QWidget *parent)
{
	Q_UNIMPLEMENTED();
	return object;
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
	//TODO load settings file
}
