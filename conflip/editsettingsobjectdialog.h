#ifndef EDITSETTINGSOBJECTDIALOG_H
#define EDITSETTINGSOBJECTDIALOG_H

#include "settingsdatabase.h"
#include "settingsfilemodel.h"

#include <QDialog>

namespace Ui {
class EditSettingsObjectDialog;
}

class EditSettingsObjectDialog : public QDialog
{
	Q_OBJECT

public:
	static SettingsObject *createObject(QObject *parent = nullptr, QWidget *window = nullptr);
	static SettingsObject *editObject(SettingsObject *object, QWidget *parent = nullptr);

private slots:
	void on_openButton_clicked();
	void on_applyButton_clicked();

private:
	Ui::EditSettingsObjectDialog *ui;
	SettingsFileModel *model;

	explicit EditSettingsObjectDialog(QWidget *parent = nullptr);
	~EditSettingsObjectDialog();

};

#endif // EDITSETTINGSOBJECTDIALOG_H
