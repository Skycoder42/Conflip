#include "editsettingsobjectdialog.h"
#include "ui_editsettingsobjectdialog.h"
#include <dialogmaster.h>
#include <QStandardPaths>
#include <QPushButton>
#include <QFontDatabase>
#include "pluginloader.h"

const QString EditSettingsObjectDialog::FileType = QStringLiteral("file");

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

	ui->textBrowser->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
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

QString EditSettingsObjectDialog::currentType() const
{
	return ui->settingsTypeComboBox->currentData().toString();
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
	if(currentType() == FileType)
		entries.append({{QStringLiteral("data")}, false});
	else
		entries = model->extractEntries();

	if(isCreate) {
		object = store->createObject(currentType(),
									 ui->pathIDLineEdit->text(),
									 entries);
	} else {
		object = store->updateObject(object,
									 ui->pathIDLineEdit->text(),
									 entries);
	}

	QDialog::accept();
}

void EditSettingsObjectDialog::on_settingsTypeComboBox_currentIndexChanged(int index)
{
	ui->pathIDLineEdit->clear();
	on_applyButton_clicked();
}

void EditSettingsObjectDialog::on_openButton_clicked()
{
	auto filter = QStringLiteral("All Files (*)");
	if(currentType() != FileType)
		filter = ui->settingsTypeComboBox->currentText() + QStringLiteral(";;") + filter;

	auto path = DialogMaster::getOpenFileName(this,
											  tr("Open a settings file"),
											  QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation),
											  filter);
	if(!path.isNull()) {
		ui->pathIDLineEdit->setText(path);
		on_applyButton_clicked();
	}
}

void EditSettingsObjectDialog::on_applyButton_clicked()
{
	if(model) {
		sortModel->setSourceModel(nullptr);
		model->deleteLater();
		model = nullptr;
	}

	auto type = currentType();
	ui->stackedWidget->setCurrentIndex(type == FileType ? 1 : 0);

	auto path = ui->pathIDLineEdit->text();
	if(path.isEmpty())
		return;

	try {
		auto file = PluginLoader::createSettings(path, type, this);

		if(type == FileType)
			tryLoadPreview();
		else {
			model = new SettingsFileModel(file, this);
			model->enablePreview(ui->dataPreviewCheckBox->isChecked());
			sortModel->setSourceModel(model);
			ui->settingsTreeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

			if(!isCreate)
				model->initialize(object);
		}

		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	} catch(QException &e) {
		qWarning() << "Failed to load" << path
				   << "for type" << type
				   << "with error" << e.what();
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		DialogMaster::warning(this, tr("Failed to load settings for the given file/ID"));
	}
}

void EditSettingsObjectDialog::on_dataPreviewCheckBox_clicked(bool checked)
{
	if(model)
		model->enablePreview(checked);
	tryLoadPreview();
}

void EditSettingsObjectDialog::setup()
{
	if(isCreate)
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	else {
		ui->settingsTypeLabel->setEnabled(false);
		ui->settingsTypeComboBox->setEnabled(false);
		ui->pathIDLineEdit->setText(object.devicePath());
		on_applyButton_clicked();
	}
}

void EditSettingsObjectDialog::tryLoadPreview()
{
	if(ui->dataPreviewCheckBox->isChecked()) {
		auto path = ui->pathIDLineEdit->text();
		if(!path.isEmpty()) {
			QFile file(path);
			if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
				ui->textBrowser->setPlainText(QString::fromUtf8(file.readAll()));
				file.close();
			} else {
				qCritical() << "Unable to open file" << path
							<< "with error:" << file.errorString();
				ui->textBrowser->setHtml(tr("<font color=\"red\">Unable to open file for reading!</font>"));
			}
		}
	} else
		ui->textBrowser->clear();
}
