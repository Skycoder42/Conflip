#include "syncengine.h"
#include <QCoreApplication>
#include <QDebug>
#include <QSaveFile>
#include <chrono>
#include <conflip.h>
#include <settings.h>

using namespace std::chrono;

SyncEngine::SyncEngine(QObject *parent) :
	QObject{parent},
	_timer{new QTimer{this}},
	_watcher{new QFileSystemWatcher{this}},
	_serializer{new QJsonSerializer{this}},
	_resolver{new PathResolver{this}},
	_threadPool{new QThreadPool{this}}
{
	if(!Settings::instance()->engine.machineid.isSet())
		Settings::instance()->engine.machineid = QUuid::createUuid();

	connect(_timer, &QTimer::timeout,
			this, &SyncEngine::triggerSync);
	connect(_watcher, &QFileSystemWatcher::fileChanged,
			this, &SyncEngine::triggerSync);
}

bool SyncEngine::start()
{
	_workingDir = QDir::cleanPath(Settings::instance()->engine.dir);
	_workingDir.makeAbsolute();
	if(!Conflip::initConfDir())
		return false;

	_resolver->setSyncDir(_workingDir);
	_timer->setInterval(minutes(Settings::instance()->engine.interval));
	resume();
	return true;
}

void SyncEngine::pause()
{
	_timer->stop();
	_threadPool->clear();
	_threadPool->waitForDone();
}

void SyncEngine::resume()
{
	_timer->start();
	QMetaObject::invokeMethod(this, "triggerSync", Qt::QueuedConnection);
}

void SyncEngine::reload()
{
	Settings::instance()->accessor()->sync();
	_workingDir = QDir::cleanPath(Settings::instance()->engine.dir);
	_workingDir.makeAbsolute();
	if(Conflip::initConfDir()) {
		_resolver->setSyncDir(_workingDir);
		_timer->setInterval(minutes(Settings::instance()->engine.interval));
		pause();
		resume();
	}
}

void SyncEngine::triggerSync()
{
	if(_skipNextUpdate) {
		_skipNextUpdate = false;
		return;
	}

	QFile readFile(_workingDir.absoluteFilePath(Conflip::ConfigFileName()));
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
		_currentDb = _serializer->deserializeFrom<ConflipDatabase>(&readFile);
		readFile.close();
		_dbChanged = false;
		_hasErrors = false;
		syncEntries(_currentDb.entries);
		removeUnsynced(_currentDb.unsynced);
	} catch (QJsonSerializerException &e) {
		qCritical() << "Failed to parse" << readFile.fileName()
					<< "with error:\n" << e.what();
		return;
	}
}

void SyncEngine::syncDone(SyncTask *task, SyncTask::Result result)
{
	auto entry = _activeTasks.take(task);
	if(!entry)
		return;

	switch(result) {
	case SyncTask::NotASymlink:
		entry->mode = QStringLiteral("copy");
		_dbChanged = true;
		Q_FALLTHROUGH(); //TODO check if ok so
	case SyncTask::Synced:
		// if sync successful and not first used yet -> mark first used
		if(!entry->syncedMachines.contains(Settings::instance()->engine.machineid)) {
			entry->syncedMachines.append(Settings::instance()->engine.machineid);
			_dbChanged = true;
		}
		break;
	case SyncTask::Removed:
		entry->syncedMachines.removeAll(Settings::instance()->engine.machineid);
		if(entry->syncedMachines.isEmpty()) {
			if(!_currentDb.unsynced.removeOne(*entry)) {
				qWarning().noquote() << "Failed to remove unsynced sync entry from database. Sync entry path: "
									 << entry->pathPattern;
			}
			entry = nullptr;
		}
		_dbChanged = true;
		break;
	case SyncTask::Error:
		_hasErrors = true;
		break;
	default:
		Q_UNREACHABLE();
		break;
	}

	if(_activeTasks.isEmpty())
		completeSync();
	task->deleteLater();
}

SyncHelper *SyncEngine::getHelper(const QString &type)
{
	auto helper = _helpers.value(type);
	if(!helper) {
		try {
			helper = Conflip::loadHelper(type, this);
			if(!helper) {
				qCritical() << "No plugin found to load helper of type" << type;
				return nullptr;
			}
			_helpers.insert(type, helper);
		} catch(QException &e) {
			qCritical() << "Failed to load plugin for type" << type
						<< "with error:" << e.what();
			return nullptr;
		}
	}
	return helper;
}

void SyncEngine::syncEntries(QList<SyncEntry> &entries)
{
	for(auto &entry : entries) {
		auto helper = getHelper(entry.mode);
		Q_ASSERT_X(helper, Q_FUNC_INFO, "No helper defined for entry mode");

		//fixup matchDirs if set but not supported by helper
		if(entry.matchDirs && !helper->canSyncDirs(entry.mode)) {
			entry.matchDirs = false;
			_dbChanged = true;
		}

		QStringList paths;
		if(helper->pathIsPattern(entry.mode))
			paths = _resolver->resolvePath(entry, helper);
		else
			paths = QStringList { entry.pathPattern };
		for(const auto& path : qAsConst(paths)) {
			auto task = helper->createSyncTask(entry.mode,
											   _workingDir,
											   path,
											   entry.extras,
											   !entry.syncedMachines.contains(Settings::instance()->engine.machineid),
											   this);
			connect(task, &SyncTask::syncDone,
					this, &SyncEngine::syncDone);
			_activeTasks.insert(task, &entry);
			_threadPool->start(task);
			//TODO check for path conflicts
		}
	}
}

void SyncEngine::removeUnsynced(QList<SyncEntry> &entries)
{
	for(auto &entry : entries) {
		if(!entry.syncedMachines.contains(Settings::instance()->engine.machineid))
			continue;

		auto helper = getHelper(entry.mode);
		Q_ASSERT_X(helper, Q_FUNC_INFO, "No helper defined for entry mode");

		QStringList paths;
		if(helper->pathIsPattern(entry.mode))
			paths = _resolver->resolvePath(entry, helper);
		else
			paths = QStringList { entry.pathPattern };
		for(const auto& path : qAsConst(paths)) {
			auto task = helper->createUndoSyncTask(entry.mode, _workingDir, path, this);
			connect(task, &SyncTask::syncDone,
					this, &SyncEngine::syncDone);
			_activeTasks.insert(task, &entry);
			_threadPool->start(task);
			//TODO check for path conflicts
		}
	}
}

void SyncEngine::completeSync()
{
	auto &machineError = _currentDb.hasErrors[Settings::instance()->engine.machineid.get().toString(QUuid::WithoutBraces)];
	if(_hasErrors != machineError) {
		machineError = _hasErrors;
		_dbChanged = true;
	}

	if(_dbChanged) {
		QSaveFile writeFile(_workingDir.absoluteFilePath(Conflip::ConfigFileName()));
		if(!writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
			qCritical() << "Failed to update file" << writeFile.fileName()
						<< "with error:" << qUtf8Printable(writeFile.errorString());
		}

		try {
			_serializer->serializeTo(&writeFile, _currentDb);
			_skipNextUpdate = true;
			if(!writeFile.commit()) {
				_skipNextUpdate = false;
				qCritical() << "Failed to update file" << writeFile.fileName()
							<< "with error:" << qUtf8Printable(writeFile.errorString());
			}
		} catch (QJsonSerializerException &e) {
			writeFile.cancelWriting();
			qCritical() << "Failed to serialize data to" << writeFile.fileName()
						<< "with error:\n" << e.what();
			return;
		}
	}
}
