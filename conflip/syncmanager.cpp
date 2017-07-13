#include "syncmanager.h"

SyncManager::SyncManager(QObject *parent) :
	QObject(parent),
	_objectStore(new QtDataSync::CachingDataStore<SettingsObject, QUuid>(this)),
	_entryStore(new QtDataSync::CachingDataStore<SettingsEntry, QUuid>(this))
{}
