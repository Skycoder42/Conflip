#include <QCoreApplication>
#include "syncengine.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("conflip"));
	QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));
	QCoreApplication::setOrganizationName(QStringLiteral(COMPANY));
	QCoreApplication::setOrganizationDomain(QStringLiteral(BUNDLE));

	SyncEngine engine;
	auto res = engine.start();
	if(res == EXIT_SUCCESS)
		return a.exec();
	else
		return res;
}
