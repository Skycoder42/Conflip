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

	void objectStoreLoaded();
	void objectChanged(const QString &key, const QVariant &value);
	void objectStoreResetted();

	void valueChanged(int metaTypeId, const QString &key, bool wasDeleted);

private:
	QtDataSync::CachingDataStore<SettingsObject, QUuid> *_objectStore;
	DataStore *_dataStore;
	QHash<QUuid, SettingsFile*> _fileMap;

	QSet<QUuid> _locks;

	void loadObject(SettingsObject object, bool localReset = true);
	void updateData(const QUuid &objectId, const QStringList &keyChain, const QVariant &data);
	void applyRemoteChange(SettingsValue value);

	void updateAll(const QUuid &objectId);
	void storeEntry(const QStringList &entryChain, QUuid objectId, SettingsFile *file, const QStringList &rootChain);
	void recurseEntry(const QStringList &entryChain, QUuid objectId, SettingsFile *file, const QStringList &rootChain);

	void syncIfChanged(SettingsValue value);
};

#endif // SYNCMANAGER_H
