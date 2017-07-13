#include "datastore.h"
#include "libconflip_global.h"
#include <QGlobalStatic>

Q_GLOBAL_STATIC(DataStore, store)

//TODO global: better exception handling?

DataStore::DataStore() :
	AsyncDataStore()
{}

DataStore *DataStore::instance()
{
	return store;
}

SettingsObject DataStore::createObject(const QString &type, const QString &path, const QList<QPair<QStringList, bool> > &entries, bool syncAll)
{
	SettingsObject object;
	object.id = QUuid::createUuid();
	object.type = type;
	object.paths.insert(deviceId(), path);
	object.syncAll = syncAll;

	//insert object once to prevent unclear states
	if(!syncAll) {
		foreach(auto entryInfo, entries) {
			SettingsEntry entry;
			entry.keyChain = entryInfo.first;
			entry.recursive = entryInfo.second;
			object.entries.append(entry);
		}
	}

	//now update with all it's entries
	save(object);

	return object;
}

SettingsObject DataStore::updateObject(SettingsObject object, const QString &path, const QList<QPair<QStringList, bool> > &entries, bool syncAll)
{
	//is actually "recreate"
	removeObject(object.id);
	return createObject(object.type, path, entries, syncAll);
}

void DataStore::removeObject(QUuid objectId)
{
	load<SettingsObject>(objectId).onResult(this, [this](SettingsObject o){
		removeObject(o);
	});
}

void DataStore::removeObject(SettingsObject object)
{
	auto objId = object.id;
	emit lockObject(objId);
	objectValues(object).onResult(this, [this, objId](QList<SettingsValue> values){
		foreach(auto value, values)
			remove<SettingsValue>(value.id());
		remove<SettingsObject>(objId);
	});
}

QtDataSync::GenericTask<QList<SettingsValue> > DataStore::objectValues(QUuid objectId)
{
	return search<SettingsValue>(objectId.toString() + QStringLiteral("-*"));
}

QtDataSync::GenericTask<QList<SettingsValue> > DataStore::objectValues(SettingsObject object)
{
	return objectValues(object.id);
}
