#include "synclogger.h"
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <iostream>
#include <QDesktopServices>
#include <QUrl>

SyncLogger * SyncLogger::_instance = nullptr;

SyncLogger::SyncLogger(bool verbose, QObject *parent) :
	QObject(parent),
	_logFile(new QFile(this)),
	_stream(),
	_verbose(verbose)
{
	QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
	dir.mkpath(QStringLiteral("."));
	_logFile->setFileName(dir.absoluteFilePath(QStringLiteral("synclog.txt")));

	if(_logFile->open(QIODevice::Append | QIODevice::Text)) {
		_stream.setDevice(_logFile);
		qInstallMessageHandler(&SyncLogger::handler);
	} else
		qCritical() << "Failed to setup up logfile!";
}

SyncLogger::~SyncLogger()
{
	_stream.flush();
	_logFile->flush();
	_logFile->close();
}

void SyncLogger::setup(bool verbose)
{
	if(!_instance)
		_instance = new SyncLogger(verbose, qApp);
}

void SyncLogger::openLogfile()
{
	_instance->_stream.flush();
	_instance->_logFile->flush();

	QDesktopServices::openUrl(QUrl::fromLocalFile(_instance->_logFile->fileName()));
}

void SyncLogger::handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	auto fmsg = qFormatLogMessage(type, context, msg);

	//write msg to file
	if(type > QtDebugMsg || _instance->_verbose)
		_instance->_stream << fmsg << QLatin1Char('\n');

	//write to stderr
	std::cerr << fmsg.toStdString() << std::endl;
}
