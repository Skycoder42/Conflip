#include <QCoreApplication>
#include <QCommandLineParser>
#include "syncengine.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("conflip"));
	QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));
	QCoreApplication::setOrganizationName(QStringLiteral(COMPANY));
	QCoreApplication::setOrganizationDomain(QStringLiteral(BUNDLE));

	QCommandLineParser parser;
	parser.setApplicationDescription(QStringLiteral("The daemon to perform configuration synchronisations"));

	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOption({
						 QStringLiteral("systemd-log"),
						 QStringLiteral("Log in a format that systemd can easily interpret without redundant information")
					 });
	parser.addOption({
						 {QStringLiteral("s"), QStringLiteral("slice")},
						 QStringLiteral("Run in slice mode. The instance will use it's own settings based on the <slice> passed."),
						 QStringLiteral("slice")
					 });
	parser.process(a);

	if(parser.isSet(QStringLiteral("slice")))
		QCoreApplication::setApplicationName(QStringLiteral("conflip@%1").arg(parser.value(QStringLiteral("slice"))));

	if(parser.isSet(QStringLiteral("systemd-log"))) {
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
