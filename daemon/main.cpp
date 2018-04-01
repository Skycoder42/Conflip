#include <QCoreApplication>
#include "syncengine.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("conflip"));
	QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));
	QCoreApplication::setOrganizationName(QStringLiteral(COMPANY));
	QCoreApplication::setOrganizationDomain(QStringLiteral(BUNDLE));

	//TODO support slicing via parameter
	if(false) { //TODO implement
		qSetMessagePattern(QStringLiteral("%{if-fatal}<0>%{endif}"
										  "%{if-critical}<2>%{endif}"
										  "%{if-warning}<4>%{endif}"
										  "%{if-info}<6>%{endif}"
										  "%{if-debug}<7>%{endif}"
										  "%{if-category}%{category}: %{endif}"
										  "%{message}"));
	} else {
		qSetMessagePattern(QStringLiteral("[%{time} %{type}]\t"
										  "%{if-category}%{category}: %{endif}"
										  "%{message}"));
	}


	SyncEngine engine;
	auto res = engine.start();
	if(res == EXIT_SUCCESS)
		return a.exec();
	else
		return res;
}
