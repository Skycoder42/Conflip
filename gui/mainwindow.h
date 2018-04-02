#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override;

private slots:
	void on_action_Reload_Daemon_triggered();
	void on_action_Add_Entry_triggered();
	void on_action_Edit_Entry_triggered();
	void on_action_Remove_Entry_triggered();
	void on_action_About_triggered();

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
