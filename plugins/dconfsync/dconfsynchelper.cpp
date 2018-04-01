#include "dconfsynchelper.h"
#include "dconfaccess.h"

#include <QSaveFile>
#include <QStandardPaths>

const QString DConfSyncHelper::ModeDConf = QStringLiteral("dconf");

DConfSyncHelper::DConfSyncHelper(QObject *parent) :
	SyncHelper(parent),
	_serializer(new QJsonSerializer(this))
{
	QJsonSerializer::registerAllConverters<DConfEntry>();
}

bool DConfSyncHelper::pathIsPattern(const QString &mode) const
{
	return false;
}

void DConfSyncHelper::performSync(const QString &path, const QString &mode, const QStringList &extras, bool isFirstUse)
{
	if(mode != ModeDConf)
		throw SyncException("Unsupported path mode");
	if(!path.startsWith('/') || !path.endsWith('/'))
		throw SyncException("DConf path must start and end with a /");

	//generate sync path and create parent dirs
	QString cnfPath = QStringLiteral("dconf") + path + QStringLiteral("dconf.json");
	QFileInfo syncInfo(QDir::cleanPath(syncDir().absoluteFilePath(cnfPath)));
	syncInfo.setCaching(false);
	if(!syncInfo.dir().exists()) {
		if(!syncInfo.dir().mkpath(QStringLiteral(".")))
			throw SyncException("Failed to create sync directory");
	}

	QByteArrayList subKeys;
	for(auto extra : extras)
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
	for(auto key : srcKeys) {
		auto strKey = QString::fromUtf8(key);
		auto entry = workingMap.value(key);

		// both src and sync have values
		if(!entry.type.isNull()) {
			auto srcDate = stamps->value(strKey).toDateTime();
			// on first use or if synced is newer -> use sync
			if(isFirstUse || entry.lastModified > srcDate) {
				QByteArray msg;
				if(!dconf.writeData(key, entry.type, entry.data, &msg))
					throw SyncException("Failed to save from sync to src with error: " + msg);
				stamps->setValue(strKey, entry.lastModified);
				log(path, "Updated entry in sync from src", key);
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
					log(path, "Updated entry in src from sync", key);
				} else
					log(path, "Skipping unchanged entry", key, true);
			}
			workingMap.remove(key);
		// value not already in sync
		} else {
			std::tie(entry.type, entry.data) = dconf.readData(key);
			entry.lastModified = QDateTime::currentDateTimeUtc();
			updateMap.insert(key, entry);
			syncNeedsSave = true;
			stamps->setValue(strKey, entry.lastModified);
			log(path, "Added new entry from src to sync", key);
		}
	}

	//step 3: sync all unhandeld sync changes
	for(auto it = workingMap.constBegin(); it != workingMap.constEnd(); it++) {
		QByteArray msg;
		if(!dconf.writeData(it.key(), it->type, it->data, &msg))
			throw SyncException("Failed to save from sync to src with error: " + msg);
		stamps->setValue(QString::fromUtf8(it.key()), it->lastModified);
		log(path, "Added new entry from sync to src", it.key());
	}

	//step 4: update sync file if needed
	if(syncNeedsSave)
		writeSyncConf(syncInfo, updateMap);
	else
		log(path, "No new src changes, not update sync", true);
}

QSharedPointer<QSettings> DConfSyncHelper::loadSettings(const QString &path)
{
	QDir cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
	QString subPath = QStringLiteral("dconf-stamps") + path;
	if(!cacheDir.mkpath(subPath) || !cacheDir.cd(subPath))
		throw SyncException("Failed to create timestamp cache dir");

	return QSharedPointer<QSettings>::create(cacheDir.absoluteFilePath(QStringLiteral("stamps.ini")),
											 QSettings::IniFormat);
}

DConfSyncHelper::DConfMap DConfSyncHelper::readSyncConf(const QFileInfo &file) const
{
	if(!file.exists())
		return {};

	QFile readFile(file.absoluteFilePath());
	if(!readFile.open(QIODevice::ReadOnly) ){ //open in binary mode - the synced file has linux fileendings
		throw SyncException("Failed to open sync file for reading with error: " +
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
		throw SyncException(QByteArrayLiteral("Failed to read sync file json with error:\n") +
							e.what());
	}
}

void DConfSyncHelper::writeSyncConf(const QFileInfo &file, const DConfSyncHelper::DConfMap &map)
{
	QSaveFile writeFile(file.absoluteFilePath());
	if(!writeFile.open(QIODevice::WriteOnly) ){ //open in binary mode - the synced file has linux fileendings
		throw SyncException("Failed to open sync file for writing with error: " +
							writeFile.errorString().toUtf8());
	}

	try {
		QMap<QString, DConfEntry> writeMap;
		for(auto it = map.constBegin(); it != map.constEnd(); it++)
			writeMap.insert(QString::fromUtf8(it.key()), it.value());

		_serializer->serializeTo(&writeFile, writeMap);

		if(!writeFile.commit()) {
			throw SyncException("Failed to save sync file with error: " +
								writeFile.errorString().toUtf8());
		}
	} catch(QJsonSerializationException &e) {
		writeFile.cancelWriting();
		throw SyncException(QByteArrayLiteral("Failed to read sync file json with error:\n") +
							e.what());
	}
}

void DConfSyncHelper::log(const QString &path, const char *msg, bool dbg) const
{
	(dbg ? qDebug() : qInfo()).noquote() << "DCONF-SYNC:" << path << "=>" << msg;
}

void DConfSyncHelper::log(const QString &path, const char *msg, const QByteArray &key, bool dbg) const
{
	(dbg ? qDebug() : qInfo()).noquote() << "DCONF-SYNC:" << path << "=>" << msg << ('[' + key + ']');
}



QString DConfEntry::getType() const
{
	return QString::fromUtf8(type);
}

void DConfEntry::setType(const QString &value)
{
	type = value.toUtf8();
}
