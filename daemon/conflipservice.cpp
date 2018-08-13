#include "conflipservice.h"
#include <QCommandLineParser>
#include <QLoggingCategory>

ConflipService::ConflipService(int &argc, char **argv) :
	Service(argc, argv)
{
	QCoreApplication::setApplicationName(QStringLiteral(TARGET));
	QCoreApplication::setApplicationVersion(QStringLiteral(VERSION));
	QCoreApplication::setOrganizationName(QStringLiteral(COMPANY));
	QCoreApplication::setOrganizationDomain(QStringLiteral(BUNDLE));
}

void ConflipService::setSlice(const QString &slice)
{
	QCoreApplication::setApplicationName(QStringLiteral(TARGET "@%1").arg(slice));
}

QtService::Service::CommandMode ConflipService::onStart()
{
	QCommandLineParser parser;
	parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
	parser.addOption({QStringLiteral("platform"), {}});
	parser.addOption({QStringLiteral("backend"), {}});
	parser.addOption({QStringLiteral("slice"), {}});
	parser.addOption({QStringLiteral("loglevel"), {}, QStringLiteral("level"),
#ifdef QT_NO_DEBUG
					  QString::number(3)});
#else
					  QString::number(4)});
#endif
	if(!parser.parse(QCoreApplication::arguments())) {
		qCritical().noquote() << parser.errorText();
		qApp->exit(EXIT_FAILURE);
		return Synchronous;
	}

	QString logStr;
	switch (parser.value(QStringLiteral("loglevel")).toInt()) {
	case 0:
		logStr.prepend(QStringLiteral("\n*.critical=false"));
		Q_FALLTHROUGH();
	case 1:
		logStr.prepend(QStringLiteral("\n*.warning=false"));
		Q_FALLTHROUGH();
	case 2:
		logStr.prepend(QStringLiteral("\n*.info=false"));
		Q_FALLTHROUGH();
	case 3:
		logStr.prepend(QStringLiteral("*.debug=false"));
		Q_FALLTHROUGH();
	case 4:
	default:
		break;
	}
	QLoggingCategory::setFilterRules(logStr);

	_engine = new SyncEngine{this};
	if(!_engine->start())
		qApp->exit(EXIT_FAILURE);
	return Synchronous;
}

QtService::Service::CommandMode ConflipService::onStop(int &exitCode)
{
	_engine->pause();
	exitCode = EXIT_SUCCESS;
	return Synchronous;
}

QtService::Service::CommandMode ConflipService::onReload()
{
	_engine->reload();
	return Synchronous;
}

QtService::Service::CommandMode ConflipService::onPause()
{
	_engine->pause();
	return Synchronous;
}

QtService::Service::CommandMode ConflipService::onResume()
{
	_engine->resume();
	return Synchronous;
}
