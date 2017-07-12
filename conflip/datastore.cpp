#include "datastore.h"
#include "libconflip_global.h"

DataStore::DataStore(QObject *parent) :
	AsyncDataStore(parent)
{}

SettingsObject DataStore::createNew(const QString &type, const QString &path, const QList<QPair<QStringList, bool> > &entries)
{
	SettingsObject object;
	object.id = QUuid::createUuid();
	object.type = type;
	object.paths.insert(deviceId(), path);

	//insert object once to prevent unclear states
	save(object);

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
