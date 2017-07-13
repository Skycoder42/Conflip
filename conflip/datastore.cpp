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
	emit lockObject(object.id);

	object.paths.insert(deviceId(), path);
	object.syncAll = syncAll;
	object.entries.clear();

	//discard all old values (they will be recreated based on the new entries)
	//TODO may not contain newly added ones!!!
	foreach(auto value, object.values)
		remove<SettingsValue>(value.toString());
	object.values.clear();

	if(!syncAll) {
		//generate/update new entries
		foreach(auto entryInfo, entries) {
			SettingsEntry entry;
			entry.keyChain = entryInfo.first;
			entry.recursive = entryInfo.second;
			object.entries.append(entry);
		}
	} else
		object.entries.clear();

	//now update with all it's entries
	save(object);

	return object;
}

void DataStore::removeObject(QUuid objectId)
{
	load<SettingsObject>(objectId).onResult(this, [this](SettingsObject o){
		removeObject(o);
	});
}

void DataStore::removeObject(SettingsObject object)
{
	emit lockObject(object.id);
	foreach(auto value, object.values)
		remove<SettingsValue>(value.toString());
	remove<SettingsObject>(object.id);
}
