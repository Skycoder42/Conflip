#ifndef SYNCENTRY_H
#define SYNCENTRY_H

#include <QObject>
#include <QString>
class QDebug;

class SyncEntry
{
	Q_GADGET

	Q_PROPERTY(QString pathPattern MEMBER pathPattern)
	Q_PROPERTY(QStringList extras MEMBER extras)
	Q_PROPERTY(PathMode mode MEMBER mode)
	Q_PROPERTY(bool	includeHidden MEMBER includeHidden)

public:
	enum PathMode {
		SymlinkMode,
		CopyMode,
		QSettingsMode,
		GConfMode
	};
	Q_ENUM(PathMode)

	QString pathPattern;
	QStringList extras;
	PathMode mode = SymlinkMode;
	bool includeHidden = true;

	bool operator==(const SyncEntry &other) const;
	bool operator!=(const SyncEntry &other) const;
};

QDebug operator<<(QDebug debug, const SyncEntry &entry);

#endif // SYNCENTRY_H
