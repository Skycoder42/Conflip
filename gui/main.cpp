#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QApplication::setApplicationName(QStringLiteral(TARGET));
	QApplication::setApplicationVersion(QStringLiteral(VERSION));
	QApplication::setOrganizationName(QStringLiteral(COMPANY));
	QApplication::setOrganizationDomain(QStringLiteral(BUNDLE));
	QApplication::setApplicationDisplayName(QStringLiteral("Conflip"));
	QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("conflip")));

	auto translator = new QTranslator(qApp);
	if(translator->load(QLocale(),
						QStringLiteral("conflip"),
						QStringLiteral("_"),
						QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
		qApp->installTranslator(translator);
	}

	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addOption({
						 {QStringLiteral("s"), QStringLiteral("slice")},
						 QStringLiteral("Run in slice mode. The instance will use the settings based on the <slice> passed."),
						 QStringLiteral("slice")
					 });
	parser.process(a);
	if(parser.isSet(QStringLiteral("slice")))
		QApplication::setApplicationName(QStringLiteral(TARGET "@%1").arg(parser.value(QStringLiteral("slice"))));

	MainWindow w;
	w.show();

	return a.exec();
}
