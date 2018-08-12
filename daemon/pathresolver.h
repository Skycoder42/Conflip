#ifndef PATHRESOLVER_H
#define PATHRESOLVER_H

#include <QDir>
#include <QObject>
#include <QStringList>
#include <syncentry.h>
#include <synchelper.h>

class PathResolver : public QObject
{
	Q_OBJECT

public:
	explicit PathResolver(QObject *parent = nullptr);

	void setSyncDir(const QDir &dir);
	QStringList resolvePath(const SyncEntry &entry, SyncHelper *helper) const;

private:
	QStringList findFiles(const QDir &cd, QStringList pathList, const SyncEntry &entry) const;
	QDir findRootDir(QStringList &pathList, const SyncEntry &entry) const;
	QDir createDir(const QString &path, const SyncEntry &entry) const;

	QDir _syncDir;
};

#endif // PATHRESOLVER_H
