#include "datastore.h"
#include "libconflip_global.h"

DataStore::DataStore(QObject *parent) :
	AsyncDataStore(parent)
{}

SettingsObject DataStore::createNew(const QString &type, const QString &path, const QList<QPair<QStringList, bool> > &entries, bool syncAll)
{
	SettingsObject object;
	object.id = QUuid::createUuid();
	object.type = type;
	object.paths.insert(deviceId(), path);
	object.syncAll = syncAll;

	//insert object once to prevent unclear states
	save(object);
	if(syncAll)
		return object;

	foreach(auto entryInfo, entries) {
		SettingsEntry entry;
		entry.objectId = object.id;
		entry.keyChain = entryInfo.first;
		entry.recursive = entryInfo.second;
		object.entries.insert(entry.id());
		save(entry);
	}

	//now update with all it's entries
	save(object);

	return object;
}

SettingsObject DataStore::update(SettingsObject object, const QString &path, const QList<QPair<QStringList, bool> > &entries, bool syncAll)
{
	object.paths.insert(deviceId(), path);
	object.syncAll = syncAll;

	if(syncAll) {
		object.entries.clear();
		save(object);
		return object;
	}

	//generate/update new entries
	QSet<QUuid> newEntries;
	foreach(auto entryInfo, entries) {
		SettingsEntry entry;
		entry.objectId = object.id;
		entry.keyChain = entryInfo.first;
		entry.recursive = entryInfo.second;
		newEntries.insert(entry.id());
		save(entry);
	}

	//get entries to delete
	auto delEntries = object.entries.subtract(newEntries);
	object.entries = newEntries;

	//now update with all it's entries
	save(object);

	//and delete the old entries
	foreach(auto entry, entries)
		remove<SettingsEntry>(entry);

	return object;
}
