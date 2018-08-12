#include "dconfsynchelper.h"
#include "dconfaccess.h"

#include <QSaveFile>
#include <QStandardPaths>

const QString DConfSyncHelper::ModeDConf = QStringLiteral("dconf");

DConfSyncHelper::DConfSyncHelper(QObject *parent) :
	SyncHelper(parent)
{}

bool DConfSyncHelper::pathIsPattern(const QString &mode) const
{
	Q_UNUSED(mode)
	return false;
}

bool DConfSyncHelper::canSyncDirs(const QString &mode) const
{
	Q_UNUSED(mode)
	return false;
}

SyncHelper::ExtrasHint DConfSyncHelper::extrasHint() const
{
	return {
		true,
		tr("Keys"),
		tr("Enter the keys you want to synchronize. All entries that start with the given keys will be synchronized.")
	};
}

SyncTask *DConfSyncHelper::createSyncTask(QString mode, const QDir &syncDir, QString path, QStringList extras, bool isFirstUse, QObject *parent)
{
	return new DConfSyncTask {
		this,
		std::move(mode),
		syncDir,
		std::move(path),
		std::move(extras),
		isFirstUse,
		parent
	};
}

SyncTask *DConfSyncHelper::createUndoSyncTask(QString mode, const QDir &syncDir, QString path, QObject *parent)
{
	return new DConfSyncTask {
		this,
		std::move(mode),
		syncDir,
		std::move(path),
		parent
	};
}



DConfSyncTask::DConfSyncTask(const DConfSyncHelper *helper, QString &&mode, const QDir &syncDir, QString &&path, QStringList &&extras, bool isFirstUse, QObject *parent) :
	SyncTask{helper, std::move(mode), syncDir, std::move(path), std::move(extras), isFirstUse, parent},
	_serializer{new QJsonSerializer{this}}
{}

DConfSyncTask::DConfSyncTask(const DConfSyncHelper *helper, QString &&mode, const QDir &syncDir, QString &&path, QObject *parent) :
	SyncTask{helper, std::move(mode), syncDir, std::move(path), parent},
	_serializer{new QJsonSerializer{this}}
{}

void DConfSyncTask::performSync()
{
	if(!path.startsWith('/') || !path.endsWith('/'))
		fatal("DConf path must start and end with a /");

	//generate sync path and create parent dirs
	auto syncInfo = syncFile();

	QByteArrayList subKeys;
	subKeys.reserve(extras.size());
	for(const auto& extra : extras)
		subKeys.append(extra.toUtf8());

	// step 1: load the current config, prepare dconf and the timestamp cache
	auto stamps = loadSettings(path);
	auto updateMap = readSyncConf(syncInfo);
	auto workingMap = updateMap;
	DConfAccess dconf;
	dconf.open(path.toUtf8());

	// step 2: update the src and sync from each other
	auto syncNeedsSave = false;
	auto srcKeys = dconf.readAllKeys(subKeys);
	for(const auto& key : srcKeys) {
		auto strKey = QString::fromUtf8(key);
		auto entry = workingMap.value(key);

		// both src and sync have values
		if(!entry.type.isNull()) {
			auto srcDate = stamps->value(strKey).toDateTime();
			// on first use or if synced is newer -> use sync
			if(isFirstUse || entry.lastModified > srcDate) {
				QByteArray msg;
				if(!dconf.writeData(key, entry.type, entry.data, &msg))
					fatal("Failed to save from sync to src with error: " + msg);
				stamps->setValue(strKey, entry.lastModified);
				info(strKey) << "Updated entry in sync from src";
			} else {
				// check if value has changed
				QByteArray type, data;
				std::tie(type, data) = dconf.readData(key);
				// if changed -> update sync from src
				if(type != entry.type || data != entry.data) {
					entry.type = type;
					entry.data = data;
					entry.lastModified = QDateTime::currentDateTimeUtc();
					updateMap.insert(key, entry);
					syncNeedsSave = true;
					stamps->setValue(strKey, entry.lastModified);
					info(strKey) << "Updated entry in src from sync";
				} else
					debug(strKey) << "Skipping unchanged entry";
			}
			workingMap.remove(key);
		// value not already in sync
		} else {
			std::tie(entry.type, entry.data) = dconf.readData(key);
			entry.lastModified = QDateTime::currentDateTimeUtc();
			updateMap.insert(key, entry);
			syncNeedsSave = true;
			stamps->setValue(strKey, entry.lastModified);
			info(strKey) << "Added new entry from src to sync";
		}
	}

	//step 3: sync all unhandeld sync changes
	for(auto it = workingMap.constBegin(); it != workingMap.constEnd(); it++) {
		auto strKey = QString::fromUtf8(it.key());
		//check if the key should still be synced
		auto keep = false;
		for(const auto& subKey : qAsConst(subKeys)) {
			if(it.key().startsWith(subKey)) {
				keep = true;
				break;
			}
		}
		// if not -> remove it and dont sync it
		if(!keep) {
			updateMap.remove(it.key());
			syncNeedsSave = true;
			info(strKey) << "Removed non-syncable entrie from sync";
			continue;
		}

		QByteArray msg;
		if(!dconf.writeData(it.key(), it->type, it->data, &msg))
			fatal("Failed to save from sync to src with error: " + msg);
		stamps->setValue(QString::fromUtf8(it.key()), it->lastModified);
		info(strKey) << "Added new entry from sync to src";
	}

	//step 4: update sync file if needed
	if(syncNeedsSave)
		writeSyncConf(syncInfo, updateMap);
	else
		debug() << "No new src changes, not update sync";
}

void DConfSyncTask::undoSync()
{
	auto syncInfo = syncFile();
	if(syncInfo.exists()) {
		if(!QFile::remove(syncInfo.absoluteFilePath()))
			fatal("Failed to remove synced file");
		info() << "Removed file from synchronisation";
	}
}

QFileInfo DConfSyncTask::syncFile()
{
	QFileInfo syncInfo{helper->toSyncPath(mode, syncDir, path + QStringLiteral("dconf.json"))};
	syncInfo.setCaching(false);
	if(!syncInfo.dir().exists()) {
		if(!syncInfo.dir().mkpath(QStringLiteral(".")))
			fatal("Failed to create sync directory");
	}
	return syncInfo;
}

QSharedPointer<QSettings> DConfSyncTask::loadSettings(const QString &path)
{
	QDir cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
	QString subPath = QStringLiteral("dconf-stamps") + path;
	if(!cacheDir.mkpath(subPath) || !cacheDir.cd(subPath))
		fatal("Failed to create timestamp cache dir");

	return QSharedPointer<QSettings>::create(cacheDir.absoluteFilePath(QStringLiteral("stamps.ini")),
											 QSettings::IniFormat);
}

DConfSyncTask::DConfMap DConfSyncTask::readSyncConf(const QFileInfo &file) const
{
	if(!file.exists())
		return {};

	QFile readFile{file.absoluteFilePath()};
	if(!readFile.open(QIODevice::ReadOnly) ){ //open in binary mode - the synced file has linux fileendings
		fatal("Failed to open sync file for reading with error: " +
			  readFile.errorString().toUtf8());
	}
	try {
		auto strMap = _serializer->deserializeFrom<QMap<QString, DConfEntry>>(&readFile);
		readFile.close();

		DConfMap resMap;
		for(auto it = strMap.constBegin(); it != strMap.constEnd(); it++)
			resMap.insert(it.key().toUtf8(), it.value());
		return resMap;
	} catch(QJsonSerializationException &e) {
		fatal(QByteArrayLiteral("Failed to read sync file json with error: ") +
			  e.what());
	}
}

void DConfSyncTask::writeSyncConf(const QFileInfo &file, const DConfSyncTask::DConfMap &map)
{
	QSaveFile writeFile{file.absoluteFilePath()};
	if(!writeFile.open(QIODevice::WriteOnly) ){ //open in binary mode - the synced file has linux fileendings
		fatal("Failed to open sync file for writing with error: " +
			  writeFile.errorString().toUtf8());
	}

	try {
		QMap<QString, DConfEntry> writeMap;
		for(auto it = map.constBegin(); it != map.constEnd(); it++)
			writeMap.insert(QString::fromUtf8(it.key()), it.value());

		_serializer->serializeTo(&writeFile, writeMap);

		if(!writeFile.commit()) {
			fatal("Failed to save sync file with error: " +
				  writeFile.errorString().toUtf8());
		}
	} catch(QJsonSerializationException &e) {
		writeFile.cancelWriting();
		fatal(QByteArrayLiteral("Failed to read sync file json with error: ") +
			  e.what());
	}
}



QString DConfEntry::getType() const
{
	return QString::fromUtf8(type);
}

void DConfEntry::setType(const QString &value)
{
	type = value.toUtf8();
}
