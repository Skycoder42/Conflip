#include "pluginloader.h"
#include "syncmanager.h"
#include <QCoreApplication>
#include <QtDataSync/SyncController>
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
	connect(_dataStore, &DataStore::dataChanged,
			this, &SyncManager::valueChanged);

	connect(_objectStore, &QtDataSync::CachingDataStoreBase::storeLoaded,
			this, &SyncManager::objectStoreLoaded);
	connect(_objectStore, &QtDataSync::CachingDataStoreBase::dataChanged,
			this, &SyncManager::objectChanged);
	connect(_objectStore, &QtDataSync::CachingDataStoreBase::dataResetted,
			this, &SyncManager::objectStoreResetted);
}

void SyncManager::lockObject(QUuid objId)
{
	_locks.insert(objId);
}

void SyncManager::objectStoreLoaded()
{
	foreach(auto object, _objectStore->loadAll())
		loadObject(object, false);
}

void SyncManager::objectChanged(const QString &key, const QVariant &value)
{
	auto rKey = _objectStore->toKey(key);

	_locks.remove(rKey);
	//if add/update -> load object
	if(value.isValid()) {
		auto object = value.value<SettingsObject>();
		if(!_fileMap.contains(object.id))
			loadObject(object);
	} else {
		auto file = _fileMap.take(rKey);
		if(file)
			file->deleteLater();
	}
}

void SyncManager::objectStoreResetted()
{
	foreach(auto file, _fileMap)
		file->deleteLater();
	_fileMap.clear();
	_locks.clear();
	foreach(auto object, _objectStore->loadAll())
		loadObject(object);
}

void SyncManager::valueChanged(int metaTypeId, const QString &key, bool wasDeleted)
{
	if(wasDeleted || metaTypeId != qMetaTypeId<SettingsValue>())
		return;

	_dataStore->load<SettingsValue>(key).onResult(this, [this](SettingsValue value){
		//skip locked objects
		if(!_locks.contains(value.objectId)) {
			applyRemoteChange(value);
		}
	});
}

void SyncManager::loadObject(SettingsObject object, bool localReset)
{
	try {
		auto objId = object.id;

		//load file
		auto file = PluginLoader::createSettings(object.devicePath(), object.type, this);
		connect(file, &SettingsFile::settingsChanged,
				this, std::bind(&SyncManager::updateData, this, objId, std::placeholders::_1, std::placeholders::_2));
		_fileMap.insert(objId, file);

		if(localReset) {
			//create automated backup (if supported)
			QSettings settings;
			if(settings.value(QStringLiteral("backup"), true).toBool())
				file->autoBackup();

			//update local state based on remote state
			_dataStore->objectValues(object).onResult(this, [this, objId, file](QList<SettingsValue> values) {
				foreach(auto value, values)
					applyRemoteChange(value);
				updateAll(objId);
				file->watchChanges();
			});
		} else {
			//sync the local state first, and after that remote changes
			updateAll(objId);
			_dataStore->objectValues(object).onResult(this, [this, objId, file](QList<SettingsValue> values) {
				foreach(auto value, values)
					applyRemoteChange(value);
				file->watchChanges();
			});
		}
	} catch(QException &e) {
		qWarning() << "Failed to load settings for" << object.devicePath()
				   << "with error:" << e.what();
	}
}

void SyncManager::updateData(const QUuid &objectId, const QStringList &keyChain, const QVariant &data)
{
	//skip locked objects
	if(_locks.contains(objectId))
		return;

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
		syncIfChanged(value);
	}
}

void SyncManager::applyRemoteChange(SettingsValue value)
{
	auto file = _fileMap.value(value.objectId);
	if(!file)
	   return;

	auto prevValue = file->value(value.keyChain);
	if(!prevValue.isValid() || prevValue != value.value) { //only update if the value has changed
		file->setValue(value.keyChain, value.value);
		qDebug() << "updated local settings for:" << value.keyChain.join(QLatin1Char('/'));
	}
}

void SyncManager::updateAll(const QUuid &objectId)
{
	auto file = _fileMap.value(objectId);
	if(!file)
	   return;

	foreach(auto entry, _objectStore->load(objectId).entries) {
		if(file->isKey(entry.keyChain))
			storeEntry(entry.keyChain, objectId, file, entry.keyChain);
		if(entry.recursive)
			recurseEntry(entry.keyChain, objectId, file, entry.keyChain);
	}
}

void SyncManager::storeEntry(const QStringList &entryChain, QUuid objectId, SettingsFile *file, const QStringList &rootChain)
{
	SettingsValue value;
	value.objectId = objectId;
	value.keyChain = entryChain;
	value.entryChain = rootChain;
	value.value = file->value(entryChain);
	syncIfChanged(value);
}

void SyncManager::recurseEntry(const QStringList &entryChain, QUuid objectId, SettingsFile *file, const QStringList &rootChain)
{
	foreach(auto keys, file->childKeys(entryChain)){
		auto nChain = entryChain;
		nChain.append(keys);
		storeEntry(nChain, objectId, file, rootChain);
	}
	foreach(auto group, file->childGroups(entryChain)) {
		auto nChain = entryChain;
		nChain.append(group);
		recurseEntry(nChain, objectId, file, rootChain);
	}
}

void SyncManager::syncIfChanged(SettingsValue value)
{
	_dataStore->load<SettingsValue>(value.id()).onResult([this, value](SettingsValue oldValue){
		if(value != oldValue) {
			_dataStore->save(value);
			qDebug() << "synced changes to remote for:" << value.keyChain.join(QLatin1Char('/'));
		}
	}, [this, value](const QException &){
		//assume key does not exist
		_dataStore->save(value);
		qDebug() << "synced changes to remote for:" << value.keyChain.join(QLatin1Char('/'));
	});
}
