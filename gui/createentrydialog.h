#ifndef CREATEENTRYDIALOG_H
#define CREATEENTRYDIALOG_H

#include <QDialog>
#include <syncentry.h>

namespace Ui {
class CreateEntryDialog;
}

class CreateEntryDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CreateEntryDialog(QWidget *parent = nullptr);
	~CreateEntryDialog() override;

	static SyncEntry createEntry(QWidget *parent = nullptr);
	static SyncEntry editEntry(const SyncEntry &entry, QWidget *parent = nullptr);

private slots:
	void on_actionSelect_File_triggered();
	void on_actionSelect_Directory_triggered();
	void on_action_Add_Extra_triggered();
	void on_action_Remove_Extra_triggered();

private:
	Ui::CreateEntryDialog *ui;
};

#endif // CREATEENTRYDIALOG_H
