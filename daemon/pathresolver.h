#ifndef PATHRESOLVER_H
#define PATHRESOLVER_H

#include <QDir>
#include <QObject>
#include <QStringList>
#include <syncentry.h>

class PathResolver : public QObject
{
	Q_OBJECT

public:
	explicit PathResolver(QObject *parent = nullptr);

	QStringList resolvePath(const SyncEntry &entry) const;

private:
	mutable bool _scanHidden;
	mutable bool _caseSensitive;

	QStringList findFiles(const QDir &cd, QStringList pathList) const;
	QDir createDir(const QString &path) const;
};

#endif // PATHRESOLVER_H
