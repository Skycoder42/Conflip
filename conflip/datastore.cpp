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

SettingsObject DataStore::update(SettingsObject object, const QString &path, const QList<QPair<QStringList, bool> > &entries, bool syncAll)
{
	object.paths.insert(deviceId(), path);
	object.syncAll = syncAll;
	object.entries.clear();

	if(!syncAll) {
		//generate/update new entries
		foreach(auto entryInfo, entries) {
			SettingsEntry entry;
			entry.keyChain = entryInfo.first;
			entry.recursive = entryInfo.second;
			object.entries.append(entry);
		}

		//TODO get entries to delete
		//auto delEntries = object.entries.subtract(newEntries);
		//object.entries = newEntries;
	}

	//now update with all it's entries
	save(object);

	return object;
}
