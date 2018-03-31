#ifndef SYNCENTRY_H
#define SYNCENTRY_H

#include <QObject>
#include <QString>
#include <QUuid>
class QDebug;

class SyncEntry
{
	Q_GADGET

	Q_PROPERTY(QString pathPattern MEMBER pathPattern)
	Q_PROPERTY(QStringList extras MEMBER extras)
	Q_PROPERTY(PathMode mode MEMBER mode)
	Q_PROPERTY(bool	includeHidden MEMBER includeHidden)
	Q_PROPERTY(bool	caseSensitive MEMBER caseSensitive)
	Q_PROPERTY(QList<QUuid> syncedMachines MEMBER syncedMachines)

public:
	enum PathMode {
		SymlinkMode = 0,
		CopyMode = 1,
		IniMode = 2,
		GConfMode = 3
	};
	Q_ENUM(PathMode)

	QString pathPattern;
	QStringList extras;
	PathMode mode = SymlinkMode;
	bool includeHidden = true;
	bool caseSensitive = true;
	QList<QUuid> syncedMachines;

	bool operator==(const SyncEntry &other) const;
	bool operator!=(const SyncEntry &other) const;
};

QDebug operator<<(QDebug debug, const SyncEntry &entry);

#endif // SYNCENTRY_H
