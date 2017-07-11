#include <QApplication>
#include <QDebug>
#include <QPluginLoader>
#include "settingsplugin.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QPluginLoader loader("../plugins/libqsettings.so");
	qDebug() << "loaded:" << loader.load();
	if(!loader.isLoaded())
		qDebug() << loader.errorString();
	auto inst = qobject_cast<SettingsPlugin*>(loader.instance());
	if(inst) {
		qDebug() << "baum" << inst->baum();
		return EXIT_SUCCESS;
	} else
		return EXIT_FAILURE;

	return a.exec();
}
