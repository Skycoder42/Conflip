#ifndef MANAGESETTINGSDIALOG_H
#define MANAGESETTINGSDIALOG_H

#include <QDialog>
#include <QListWidget>
#include "datastore.h"

namespace Ui {
class ManageSettingsDialog;
}

class ManageSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ManageSettingsDialog(QWidget *parent = nullptr);
	~ManageSettingsDialog();

private slots:
	void on_listWidget_itemActivated(QListWidgetItem *item);
	void on_action_Add_Settings_triggered();
	void on_action_Edit_triggered();
	void on_action_Remove_triggered();

private:
	Ui::ManageSettingsDialog *ui;
	DataStore *_store;
};

#endif // MANAGESETTINGSDIALOG_H
