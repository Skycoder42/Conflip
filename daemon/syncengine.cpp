#include "syncengine.h"
#include <QCoreApplication>
#include <QDebug>
#include <chrono>
#include <settings.h>
#include "conflipdatabase.h"
using namespace std::chrono;

const QString SyncEngine::ConfigFileName(QStringLiteral("config.json"));

SyncEngine::SyncEngine(QObject *parent) :
	QObject(parent),
	_timer(new QTimer(this)),
	_watcher(new QFileSystemWatcher(this)),
	_serializer(new QJsonSerializer(this)),
	_resolver(new PathResolver(this)),
	_workingDir(static_cast<QString>(Settings::instance()->engine.dir))
{
	connect(_timer, &QTimer::timeout,
			this, &SyncEngine::triggerSync);
	connect(_watcher, &QFileSystemWatcher::fileChanged,
			this, &SyncEngine::triggerSync);

	_timer->setInterval(minutes(Settings::instance()->engine.interval));
}

int SyncEngine::start()
{
	if(!_workingDir.exists()) {
		qCritical().noquote() << "Working directory"
							  << _workingDir.absolutePath()
							  << "does not exist";
		return EXIT_FAILURE;
	}

	_timer->start();
	QMetaObject::invokeMethod(this, "triggerSync", Qt::QueuedConnection);
	return EXIT_SUCCESS;
}

void SyncEngine::triggerSync()
{
	QFile readFile(_workingDir.absoluteFilePath(ConfigFileName));
	if(!readFile.exists())
		return;

	// prepare or update the watcher
	auto files = _watcher->files();
	if(!files.contains(readFile.fileName())) {
		if(!files.isEmpty())
			_watcher->removePaths(files);
		_watcher->addPath(readFile.fileName());
	}

	if(!readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qCritical() << "Failed to open file" << readFile.fileName()
					<< "with error:" << qUtf8Printable(readFile.errorString());
		return;
	}

	try {
		auto database = _serializer->deserializeFrom<ConflipDatabase>(&readFile);
		for(auto entry : database.entries) {
			auto paths = _resolver->resolvePath(entry);
			qDebug() << entry << paths;
		}
	} catch (QException &e) {
		qCritical() << "Failed to parse" << readFile.fileName()
					<< "with error:" << e.what();
		return;
	}
}
