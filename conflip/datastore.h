#ifndef DATASTORE_H
#define DATASTORE_H

#include <QtDataSync/AsyncDataStore>
#include "settingsdatabase.h"

class DataStore : public QtDataSync::AsyncDataStore
{
	Q_OBJECT

public:
	explicit DataStore(QObject *parent = nullptr);

	SettingsObject createNew(const QString &type, const QString &path, const QList<QPair<QStringList, bool>> &entries, bool syncAll);
	SettingsObject update(SettingsObject object, const QString &path, const QList<QPair<QStringList, bool>> &entries, bool syncAll);
};

#endif // DATASTORE_H