#ifndef DATASTORE_H
#define DATASTORE_H

#include <QtDataSync/AsyncDataStore>
#include "settingsdatabase.h"

class DataStore : public QtDataSync::AsyncDataStore
{
	Q_OBJECT

public:
	DataStore();

	static DataStore *instance();

	SettingsObject createObject(const QString &type, const QString &path, const QList<QPair<QStringList, bool>> &entries, bool syncAll);
	SettingsObject updateObject(SettingsObject object, const QString &path, const QList<QPair<QStringList, bool>> &entries, bool syncAll);
	void removeObject(QUuid objectId);
	void removeObject(SettingsObject object);

signals:
	void lockObject(QUuid objId);
};

#endif // DATASTORE_H
