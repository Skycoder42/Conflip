#ifndef SYNCMANAGER_H
#define SYNCMANAGER_H

#include <QObject>
#include <QtDataSync/CachingDataStore>
#include "datastore.h"

class SyncManager : public QObject
{
	Q_OBJECT

public:
	explicit SyncManager(QObject *parent = nullptr);

private slots:

private:
	QtDataSync::CachingDataStore<SettingsObject, QUuid> *_objectStore;
	QtDataSync::CachingDataStore<SettingsEntry, QUuid> *_entryStore;
};

#endif // SYNCMANAGER_H
