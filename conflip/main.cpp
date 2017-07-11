#include <QApplication>
#include <QDebug>
#include "pluginloader.h"
#include "settingsplugin.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	PluginLoader::loadPlugins();
	qDebug() << PluginLoader::availablePlugins();
	return 0;

	return a.exec();
}
