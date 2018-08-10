#ifndef SYNCHELPER_H
#define SYNCHELPER_H

#include <QDir>
#include <QFileInfo>
#include <QException>
#include <QObject>
#include "syncentry.h"
#include "lib_conflip_global.h"

class LIBCONFLIPSHARED_EXPORT SyncHelper : public QObject
{
public:
	SyncHelper(QObject *parent = nullptr);

	void setSyncDir(const QDir &dir);

	virtual QString syncPrefix() const = 0;

	virtual bool pathIsPattern(const QString &mode) const = 0;
	virtual QString toSyncPath(const QString &path) const;
	virtual QString toSrcPath(const QString &path) const;

	virtual void performSync(const QString &path, const QString &mode, const QStringList &extras, bool isFirstUse) = 0;
	virtual void undoSync(const QString &path, const QString &mode) = 0;

protected:
	QDir syncDir() const;
	std::tuple<QFileInfo, QFileInfo> generatePaths(const QString &path) const;
	void removeSyncPath(const QString &path, const QByteArray &logPrefix);

private:
	QDir _syncDir;
};

class LIBCONFLIPSHARED_EXPORT SyncException : public QException
{
public:
	SyncException(QByteArray what);

	const char *what() const noexcept override;
	void raise() const override;
	QException *clone() const override;

private:
	const QByteArray _what;
};

class NotASymlinkException {};

#endif // SYNCHELPER_H
