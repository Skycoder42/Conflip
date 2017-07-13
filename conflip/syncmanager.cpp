#include "pluginloader.h"
#include "syncmanager.h"

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
		reloadObject(object);
}

void SyncManager::dataChanged(const QString &key, const QVariant &value)
{
	auto rKey = _objectStore->toKey(key);
	auto object = value.value<SettingsObject>();

	//unlock & reload
	auto file = _fileMap.take(rKey);
	if(file)
		file->deleteLater();

	_locks.remove(rKey);
	reloadObject(object);
}

void SyncManager::dataResetted()
{
	_locks.clear();
	qDeleteAll(_fileMap.values());//TODO deleteAllLater
	_fileMap.clear();
	foreach(auto object, _objectStore->loadAll())
		reloadObject(object);
}

void SyncManager::reloadObject(SettingsObject object)
{
	try {
		//load file
		auto file = PluginLoader::createSettings(object.devicePath(), object.type, this);


	} catch(QException &e) {
		qWarning() << "Failed to load settings for" << object.devicePath()
				   << "with error:" << e.what();
	}
}
