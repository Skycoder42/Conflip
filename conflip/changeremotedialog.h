#ifndef CHANGEREMOTEDIALOG_H
#define CHANGEREMOTEDIALOG_H

#include <QAbstractButton>
#include <QDialog>
#include <QtDataSync/WsAuthenticator>

namespace Ui {
class ChangeRemoteDialog;
}

class ChangeRemoteDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ChangeRemoteDialog(QWidget *parent = nullptr);
	~ChangeRemoteDialog();

	void accept() override;

private slots:
	void on_serverSecretCheckBox_toggled(bool checked);
	void on_buttonBox_clicked(QAbstractButton *button);
	void updateButton();

private:
	Ui::ChangeRemoteDialog *ui;
	QtDataSync::WsAuthenticator *_auth;
};

#endif // CHANGEREMOTEDIALOG_H
