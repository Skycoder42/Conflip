#ifndef SYNCMANAGER_H
#define SYNCMANAGER_H

#include <QObject>
#include <QtDataSync/CachingDataStore>
#include "datastore.h"
#include "settingsfile.h"

class SyncManager : public QObject
{
	Q_OBJECT

public:
	explicit SyncManager(QObject *parent = nullptr);

private slots:
	void lockObject(QUuid objId);

	void storeLoaded();
	void dataChanged(const QString &key, const QVariant &value);
	void dataResetted();

private:
	QtDataSync::CachingDataStore<SettingsObject, QUuid> *_objectStore;
	DataStore *_dataStore;
	QHash<QUuid, SettingsFile*> _fileMap;

	QHash<QUuid, int> _loadCache;
	QSet<QUuid> _locks;

	void reloadObject(SettingsObject object);
	void completeObjectSetup(const QUuid &objectId);
	void updateData(const QUuid &objectId, const QStringList &keyChain, const QVariant &data);
	void applyRemoteChange(SettingsValue value);

	void updateAll(const QUuid &objectId);
};

#endif // SYNCMANAGER_H
