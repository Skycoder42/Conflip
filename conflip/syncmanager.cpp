#include "pluginloader.h"
#include "syncmanager.h"
#include <functional>

SyncManager::SyncManager(QObject *parent) :
	QObject(parent),
	_objectStore(new QtDataSync::CachingDataStore<SettingsObject, QUuid>(this)),
	_dataStore(DataStore::instance()),
	_fileMap(),
	_loadCache(),
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
		_fileMap.insert(object.id, file);

		//update local state based on remote state
		foreach(auto value, object.values) {
			_loadCache[object.id]++;
			_dataStore->load<SettingsValue>(value.toString()).onResult(this, [this](SettingsValue value){
				applyRemoteChange(value);
				if(--_loadCache[value.objectId] == 0)//contine when the last one was loaded
					completeObjectSetup(value.objectId);
			});
		}

		//if no local state -> compelte
		if(object.values.isEmpty())
			completeObjectSetup(object.id);
	} catch(QException &e) {
		qWarning() << "Failed to load settings for" << object.devicePath()
				   << "with error:" << e.what();
	}
}

void SyncManager::completeObjectSetup(const QUuid &objectId)
{
	auto file = _fileMap.value(objectId);
	if(!file)
	   return;

	//connect for changes
	auto updateFn = std::bind(&SyncManager::updateData, this, objectId, std::placeholders::_1, std::placeholders::_2);
	connect(file, &SettingsFile::settingsChanged,
			this, updateFn);

	//load all data from settings
	updateAll(objectId);
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
		//TODO update object
		qDebug() << "synced" << keyChain << "with" << data;
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
	Q_UNIMPLEMENTED();
}
