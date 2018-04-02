#ifndef CONFLIPDATABASE_H
#define CONFLIPDATABASE_H

#include <QObject>
#include <QList>
#include "syncentry.h"
#include "lib_conflip_global.h"

class LIBCONFLIPSHARED_EXPORT ConflipDatabase
{
	Q_GADGET

	Q_PROPERTY(QList<SyncEntry> entries MEMBER entries)
	Q_PROPERTY(QList<SyncEntry> unsynced MEMBER unsynced)

public:
	QList<SyncEntry> entries;
	QList<SyncEntry> unsynced;
};

#endif // CONFLIPDATABASE_H
