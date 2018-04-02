#ifndef SYNCENTRY_H
#define SYNCENTRY_H

#include <QObject>
#include <QString>
#include <QUuid>
class QDebug;

#include "lib_conflip_global.h"

class LIBCONFLIPSHARED_EXPORT SyncEntry
{
	Q_GADGET

	Q_PROPERTY(QString pathPattern MEMBER pathPattern)
	Q_PROPERTY(QStringList extras MEMBER extras)
	Q_PROPERTY(QString mode MEMBER mode)
	Q_PROPERTY(bool	includeHidden MEMBER includeHidden)
	Q_PROPERTY(bool	caseSensitive MEMBER caseSensitive)
	Q_PROPERTY(QList<QUuid> syncedMachines MEMBER syncedMachines)

public:
	QString pathPattern;
	QStringList extras;
	QString mode;
	bool includeHidden = true;
	bool caseSensitive = true;
	QList<QUuid> syncedMachines;

	bool operator==(const SyncEntry &other) const;
	bool operator!=(const SyncEntry &other) const;

	operator bool() const;
	bool operator!() const;
};

LIBCONFLIPSHARED_EXPORT QDebug operator<<(QDebug debug, const SyncEntry &entry);

#endif // SYNCENTRY_H
