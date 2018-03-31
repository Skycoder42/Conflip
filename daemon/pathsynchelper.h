#ifndef PATHSYNCHELPER_H
#define PATHSYNCHELPER_H

#include <QCache>
#include <QObject>
#include <QRegularExpression>
#include "synchelper.h"

class PathSyncHelper : public QObject, public SyncHelper
{
	Q_OBJECT
	Q_INTERFACES(SyncHelper)

public:
	explicit PathSyncHelper(QObject *parent = nullptr);

	void performSync(const QString &path, SyncEntry::PathMode mode, const QStringList &extras, bool isFirstUse) override;

private:
	mutable QCache<QString, QRegularExpression> _regexCache;

	void syncAsSymlink(const QFileInfo &src, const QFileInfo &sync, bool isFirstUse);
	void syncAsCopy(const QFileInfo &src, const QFileInfo &sync, bool isFirstUse);

	QByteArray hashFile(const QFileInfo &file) const;
	void log(const QFileInfo &file, const char *msg, bool dbg = false) const;
};

class NotASymlinkException {};

#endif // PATHSYNCHELPER_H
