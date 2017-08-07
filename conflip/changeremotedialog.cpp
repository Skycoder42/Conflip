#include "changeremotedialog.h"
#include "ui_changeremotedialog.h"
#include <QPushButton>
#include <QtDataSync/Setup>
#include <dialogmaster.h>
#include <qurlvalidator.h>

ChangeRemoteDialog::ChangeRemoteDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ChangeRemoteDialog),
	_auth(QtDataSync::Setup::authenticatorForSetup<QtDataSync::WsAuthenticator>(this))
{
	ui->setupUi(this);
	DialogMaster::masterDialog(this);

	ui->remoteURLLineEdit->setValidator(new QUrlValidator({QStringLiteral("ws"), QStringLiteral("wss")}, this));
	connect(ui->remoteURLLineEdit, &QLineEdit::textChanged,
			this, &ChangeRemoteDialog::updateButton);

	ui->remoteURLLineEdit->setText(_auth->remoteUrl().toString());
}

ChangeRemoteDialog::~ChangeRemoteDialog()
{
	delete ui;
}

void ChangeRemoteDialog::accept()
{
	_auth->setRemoteUrl(ui->remoteURLLineEdit->text());
	if(ui->serverSecretCheckBox->isChecked())
		_auth->setServerSecret(ui->serverSecretLineEdit->text());
	_auth->reconnect();
	QDialog::accept();
}

void ChangeRemoteDialog::on_serverSecretCheckBox_toggled(bool checked)
{
	if(checked)
		ui->serverSecretLineEdit->setPlaceholderText(QString());
	else
		ui->serverSecretLineEdit->setPlaceholderText(tr("Keep current secret"));
}

void ChangeRemoteDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if(ui->buttonBox->standardButton(button) == QDialogButtonBox::RestoreDefaults) {
		ui->remoteURLLineEdit->setText(QStringLiteral("wss://apps.skycoder42.de/conflip/"));
		ui->serverSecretCheckBox->setChecked(true);
		ui->serverSecretLineEdit->setText(QStringLiteral("baum42")); //TODO debug
		accept();
	}
}

void ChangeRemoteDialog::updateButton()
{
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->remoteURLLineEdit->hasAcceptableInput());
}
