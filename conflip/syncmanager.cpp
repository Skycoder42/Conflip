#include "pluginloader.h"
#include "syncmanager.h"
#include <QCoreApplication>
#include <functional>

SyncManager::SyncManager(QObject *parent) :
	QObject(parent),
	_objectStore(new QtDataSync::CachingDataStore<SettingsObject, QUuid>(this)),
	_dataStore(DataStore::instance()),
	_fileMap(),
	_locks()
{
	connect(_dataStore, &DataStore::lockObject,
			this, &SyncManager::lockObject);

	connect(_objectStore, &QtDataSync::CachingDataStoreBase::storeLoaded,
			this, &SyncManager::storeLoaded);
	connect(_objectStore, &QtDataSync::CachingDataStoreBase::dataChanged,
			this, &SyncManager::dataChanged);
	connect(_objectStore, &QtDataSync::CachingDataStoreBase::dataResetted,
			this, &SyncManager::dataResetted);
}

void SyncManager::lockObject(QUuid objId)
{
	_locks.insert(objId);
}

void SyncManager::storeLoaded()
{
	foreach(auto object, _objectStore->loadAll())
		loadObject(object);
}

void SyncManager::dataChanged(const QString &key, const QVariant &value)
{
	auto rKey = _objectStore->toKey(key);

	//delete file (if present) and unlock
	auto file = _fileMap.take(rKey);
	if(file)
		file->deleteLater();
	_locks.remove(rKey);

	//if add/update -> load object
	if(value.isValid()) {
		auto object = value.value<SettingsObject>();
		loadObject(object);
	}
}

void SyncManager::dataResetted()
{
	qDeleteAll(_fileMap.values());//TODO deleteAllLater
	_fileMap.clear();
	_locks.clear();
	foreach(auto object, _objectStore->loadAll())
		loadObject(object);
}

void SyncManager::loadObject(SettingsObject object)
{
	try {
		auto objId = object.id;

		//load file
		auto file = PluginLoader::createSettings(object.devicePath(), object.type, this);
		connect(file, &SettingsFile::settingsChanged,
				this, std::bind(&SyncManager::updateData, this, objId, std::placeholders::_1, std::placeholders::_2));
		_fileMap.insert(objId, file);

		//update local state based on remote state
		_dataStore->objectValues(object).onResult(this, [this, objId, file](QList<SettingsValue> values) {
			foreach(auto value, values)
				applyRemoteChange(value);
			updateAll(objId);
			file->watchChanges();
		});
	} catch(QException &e) {
		qWarning() << "Failed to load settings for" << object.devicePath()
				   << "with error:" << e.what();
	}
}

void SyncManager::updateData(const QUuid &objectId, const QStringList &keyChain, const QVariant &data)
{
	if(keyChain.isEmpty())
		updateAll(objectId);
	else {
		auto file = _fileMap.value(objectId);
		if(!file)
		   return;

		//prepare value
		SettingsValue value;
		value.objectId = objectId;
		value.keyChain = keyChain;

		//find an entry that matches
		auto found = false;
		foreach(auto entry, _objectStore->load(objectId).entries) {
			if(entry.recursive)
				found = (keyChain.mid(0, entry.keyChain.size()) == entry.keyChain);
			else
				found = (keyChain == entry.keyChain);
			if(found) {
				value.entryChain = entry.keyChain;
				break;
			}
		}

		//value does not match -> do not sync
		if(!found)
			return;

		//load data from settings if not valid
		if(data.isValid())
			value.value = data;
		else
			value.value = file->value(keyChain);

		//store value
		_dataStore->save(value);
		qDebug() << "synced" << value.keyChain << "with" << value.value;
	}
}

void SyncManager::applyRemoteChange(SettingsValue value)
{
	auto file = _fileMap.value(value.objectId);
	if(!file)
	   return;
	file->setValue(value.keyChain, value.value);
	qDebug() << "updated" << value.keyChain << "to" << value.value;
}

void SyncManager::updateAll(const QUuid &objectId)
{
	auto file = _fileMap.value(objectId);
	if(!file)
	   return;

	auto object = _objectStore->load(objectId);
	foreach(auto entry, object.entries) {
		storeEntry(entry.keyChain, objectId, file, entry.keyChain);
		if(entry.recursive)
			recurseEntry(entry.keyChain, objectId, file, entry.keyChain);
	}

	qDebug() << "updated all";
}

void SyncManager::storeEntry(const QStringList &entryChain, QUuid objectId, SettingsFile *file, const QStringList &rootChain)
{
	if(file->isKey(entryChain)) {
		SettingsValue value;
		value.objectId = objectId;
		value.keyChain = entryChain;
		value.entryChain = rootChain;
		value.value = file->value(entryChain);
		_dataStore->save(value);
		qDebug() << "synced" << value.keyChain << "with" << value.value;
	}
}

void SyncManager::recurseEntry(const QStringList &entryChain, QUuid objectId, SettingsFile *file, const QStringList &rootChain)
{
	foreach(auto group, file->childGroups(entryChain)) {
		auto nChain = entryChain;
		nChain.prepend(group);
		storeEntry(nChain, objectId, file, rootChain);
		recurseEntry(nChain, objectId, file, rootChain);
	}
}
