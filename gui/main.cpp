#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QApplication::setApplicationName(QStringLiteral("conflip"));
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

	MainWindow w;
	w.show();

	return a.exec();
}
