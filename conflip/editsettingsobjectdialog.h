#ifndef EDITSETTINGSOBJECTDIALOG_H
#define EDITSETTINGSOBJECTDIALOG_H

#include "settingsdatabase.h"
#include "settingsfilemodel.h"
#include "datastore.h"

#include <QDialog>
#include <QSortFilterProxyModel>

namespace Ui {
class EditSettingsObjectDialog;
}

class EditSettingsObjectDialog : public QDialog
{
	Q_OBJECT

public:
	static SettingsObject createObject(QWidget *parent = nullptr);
	static SettingsObject editObject(SettingsObject object, QWidget *parent = nullptr);

	void accept() override;

private slots:
	void on_openButton_clicked();
	void on_dataPreviewCheckBox_clicked(bool checked);

	void loadFile();

private:
	static const QString FileType;

	Ui::EditSettingsObjectDialog *ui;
	QHash<QString, int> indexMap;
	SettingsFileModel *model;
	QSortFilterProxyModel *sortModel;
	DataStore *store;

	bool isCreate;
	SettingsObject object;

	explicit EditSettingsObjectDialog(QWidget *parent = nullptr);
	~EditSettingsObjectDialog();

	QString currentType() const;
	void setup();
	void tryLoadPreview(bool checked);
	void clearPreview();
};

#endif // EDITSETTINGSOBJECTDIALOG_H
