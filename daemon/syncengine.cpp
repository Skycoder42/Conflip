#include "syncengine.h"
#include <QCoreApplication>
#include <QDebug>
#include <QSaveFile>
#include <chrono>
#include <settings.h>
#include "conflipdatabase.h"
#include "dconfsynchelper.h"
#include "inisynchelper.h"
#include "pathsynchelper.h"
using namespace std::chrono;

const QString SyncEngine::ConfigFileName(QStringLiteral("config.json"));

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

	auto pathHelper = new PathSyncHelper(this);
	_helpers.insert(SyncEntry::SymlinkMode, pathHelper);
	_helpers.insert(SyncEntry::CopyMode, pathHelper);
	_helpers.insert(SyncEntry::IniMode, new IniSyncHelper(this));
	_helpers.insert(SyncEntry::DConfMode, new DConfSyncHelper(this));
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

	for(auto helper : _helpers)
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
		readFile.close();
		auto changed = false;

		for(auto &entry : database.entries) {
			auto helper = _helpers.value(entry.mode);
			Q_ASSERT_X(helper, Q_FUNC_INFO, "No helper defined for entry mode");

			auto paths = _resolver->resolvePath(entry);
			for(auto path : paths) {
				auto isFirst = !entry.syncedMachines.contains(Settings::instance()->engine.machineid);
				try {
					helper->performSync(path, entry.mode, entry.extras, isFirst);
				} catch(NotASymlinkException &e) {
					Q_UNUSED(e)
					entry.mode = SyncEntry::CopyMode;
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

		if(changed) {
			QSaveFile writeFile(readFile.fileName());
			if(!writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
				qCritical() << "Failed to update file" << writeFile.fileName()
							<< "with error:" << qUtf8Printable(writeFile.errorString());
			}

			_serializer->serializeTo(&writeFile, database);
			_skipNextUpdate = true;
			writeFile.commit(); //TODO print error
		}
	} catch (QJsonSerializerException &e) {
		qCritical() << "Failed to parse" << readFile.fileName()
					<< "with error:\n" << e.what();
		return;
	}
}
