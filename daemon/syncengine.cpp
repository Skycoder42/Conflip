#include "syncengine.h"
#include <QCoreApplication>
#include <QDebug>
#include <QSaveFile>
#include <chrono>
#include <QCtrlSignals>
#include <conflipdatabase.h>
#include <conflip.h>
#include <settings.h>

using namespace std::chrono;

SyncEngine::SyncEngine(QObject *parent) :
	QObject(parent),
	_timer(new QTimer(this)),
	_watcher(new QFileSystemWatcher(this)),
	_serializer(new QJsonSerializer(this)),
	_resolver(new PathResolver(this)),
	_helpers(),
	_workingDir(),
	_skipNextUpdate(false)
{
	if(!Settings::instance()->engine.machineid.isSet())
		Settings::instance()->engine.machineid = QUuid::createUuid();

	connect(_timer, &QTimer::timeout,
			this, &SyncEngine::triggerSync);
	connect(_watcher, &QFileSystemWatcher::fileChanged,
			this, &SyncEngine::triggerSync);

	QCtrlSignalHandler::instance()->setAutoQuitActive(true);
	connect(QCtrlSignalHandler::instance(), &QCtrlSignalHandler::ctrlSignal,
			this, &SyncEngine::signalTriggered);
	QCtrlSignalHandler::instance()->registerForSignal(SIGHUP);
}

int SyncEngine::start()
{
	_workingDir = static_cast<QString>(Settings::instance()->engine.dir);
	if(!_workingDir.exists()) {
		qCritical().noquote() << "Working directory"
							  << _workingDir.absolutePath()
							  << "does not exist";
		return EXIT_FAILURE;
	}

	for(auto helper : qAsConst(_helpers))
		helper->setSyncDir(_workingDir);

	_timer->start(minutes(Settings::instance()->engine.interval));
	QMetaObject::invokeMethod(this, "triggerSync", Qt::QueuedConnection);
	return EXIT_SUCCESS;
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
		auto database = _serializer->deserializeFrom<ConflipDatabase>(&readFile);
		readFile.close();
		auto changed = false;

		// synchronize entries
		syncEntries(database.entries, changed);
		//unsync
		removeUnsynced(database.unsynced, changed);

		if(changed) {
			QSaveFile writeFile(_workingDir.absoluteFilePath(Conflip::ConfigFileName()));
			if(!writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
				qCritical() << "Failed to update file" << writeFile.fileName()
							<< "with error:" << qUtf8Printable(writeFile.errorString());
			}

			_serializer->serializeTo(&writeFile, database);
			_skipNextUpdate = true;
			if(!writeFile.commit()) {
				qCritical() << "Failed to update file" << writeFile.fileName()
							<< "with error:" << qUtf8Printable(writeFile.errorString());
			}
		}
	} catch (QJsonSerializerException &e) {
		qCritical() << "Failed to parse" << readFile.fileName()
					<< "with error:\n" << e.what();
		return;
	}
}

void SyncEngine::signalTriggered(int signal)
{
	switch (signal) {
	case SIGHUP:
		_workingDir = static_cast<QString>(Settings::instance()->engine.dir);
		if(_workingDir.exists()) {
			for(auto helper : qAsConst(_helpers))
				helper->setSyncDir(_workingDir);
			triggerSync();
		}
		break;
	default:
		break;
	}
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
			helper->setSyncDir(_workingDir);
			_helpers.insert(type, helper);
		} catch(QException &e) {
			qCritical() << "Failed to load plugin for type" << type
						<< "with error:" << e.what();
			return nullptr;
		}
	}
	return helper;
}

void SyncEngine::syncEntries(QList<SyncEntry> &entries, bool &changed)
{
	for(auto &entry : entries) {
		auto helper = getHelper(entry.mode);
		Q_ASSERT_X(helper, Q_FUNC_INFO, "No helper defined for entry mode");

		QStringList paths;
		if(helper->pathIsPattern(entry.mode))
			paths = _resolver->resolvePath(entry);
		else
			paths = QStringList { entry.pathPattern };
		for(const auto& path : qAsConst(paths)) {
			auto isFirst = !entry.syncedMachines.contains(Settings::instance()->engine.machineid);
			try {
				helper->performSync(path, entry.mode, entry.extras, isFirst);
			} catch(NotASymlinkException &e) {
				Q_UNUSED(e)
				entry.mode = QStringLiteral("copy");
				changed = true;
			} catch(SyncException &e) {
				qCritical() << "ERROR:" << path
							<< "=>" << e.what();
				continue;
			}
			// if sync successful and not first used yet -> mark first used
			if(isFirst) {
				entry.syncedMachines.append(Settings::instance()->engine.machineid);
				changed = true;
			}
		}
	}
}

void SyncEngine::removeUnsynced(QList<SyncEntry> &entries, bool &changed)
{
	for(auto it = entries.begin(); it != entries.end();) {
		if(!it->syncedMachines.contains(Settings::instance()->engine.machineid))
			continue;

		auto helper = getHelper(it->mode);
		Q_ASSERT_X(helper, Q_FUNC_INFO, "No helper defined for entry mode");

		QStringList paths;
		if(helper->pathIsPattern(it->mode))
			paths = _resolver->resolvePath(*it);
		else
			paths = QStringList { it->pathPattern };
		for(const auto& path : qAsConst(paths)) {
			try {
				helper->undoSync(path, it->mode);
			} catch(SyncException &e) {
				qCritical() << "ERROR:" << path
							<< "=>" << e.what();
				return;
			}
		}

		it->syncedMachines.removeAll(Settings::instance()->engine.machineid);
		if(it->syncedMachines.isEmpty())
			it = entries.erase(it);
		else
			it++;
		changed = true;
	}
}
