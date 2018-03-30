#ifndef CONFLIPDATABASE_H
#define CONFLIPDATABASE_H

#include <QObject>
#include <QList>
#include "syncentry.h"

class ConflipDatabase
{
	Q_GADGET

	Q_PROPERTY(QList<SyncEntry> entries MEMBER entries)

public:
	QList<SyncEntry> entries;
};

#endif // CONFLIPDATABASE_H
