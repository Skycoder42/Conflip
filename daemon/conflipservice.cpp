#include "conflipservice.h"

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
