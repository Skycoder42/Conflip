#ifndef SYNCHELPER_H
#define SYNCHELPER_H

#include <QDir>
#include <QFileInfo>
#include <QException>
#include <QObject>
#include <QRunnable>
#include "syncentry.h"
#include "lib_conflip_global.h"

class SyncHelper;
class LIBCONFLIPSHARED_EXPORT SyncTask : public QObject, public QRunnable
{
	Q_OBJECT
	Q_DISABLE_COPY(SyncTask)

public:
	enum Result {
		Invalid,
		Synced,
		Removed,
		Error,
		NotASymlink
	};
	Q_ENUM(Result)

	// sync
	SyncTask(const SyncHelper *helper,
			 QString mode,
			 const QDir &syncDir,
			 QString path,
			 QStringList extras,
			 bool isFirstUse,
			 QObject *parent);
	// remove
	SyncTask(const SyncHelper *helper,
			 QString mode,
			 const QDir &syncDir,
			 QString path,
			 QObject *parent);

	void run() final;

protected:
	virtual void performSync() = 0;
	virtual void undoSync();

	QFileInfo srcPath() const;
	QFileInfo syncPath() const;

	QDebug debug(const QString &extra = {}) const;
	QDebug info(const QString &extra = {}) const;
	QDebug warning(const QString &extra = {}) const;
	QDebug critical(const QString &extra = {}) const;
	Q_NORETURN void fatal(const QString &message, const QString &extra = {}) const;
	Q_NORETURN void fatal(const QByteArray &message, const QString &extra = {}) const;
	Q_NORETURN void fatal(const char *message, const QString &extra = {}) const;

	const SyncHelper * const helper;
	const QString mode;
	const QDir syncDir;
	const QString path;
	const QStringList extras;
	const bool isFirstUse = false;

signals:
	void syncDone(SyncTask *task, SyncTask::Result result);

private slots:
	void reportResult();

private:
	const bool _isRemove;
	mutable QList<std::pair<QtMsgType, QString>> _log;
	Result _result = Invalid;

	void logBase(QDebug &dbg, const QString &extra) const;
};

class LIBCONFLIPSHARED_EXPORT SyncHelper : public QObject
{
public:
	struct ExtrasHint {
		bool enabled;
		QString title;
		QString hint;
	};

	SyncHelper(QObject *parent = nullptr);

	virtual QString syncPrefix(const QString &mode) const;

	virtual bool pathIsPattern(const QString &mode) const = 0;
	virtual bool canSyncDirs(const QString &mode) const = 0;
	virtual QString toSyncPath(const QString &mode, const QDir &syncDir, const QString &path) const;
	virtual QString toSrcPath(const QString &mode, const QDir &syncDir, const QString &path) const;

	virtual ExtrasHint extrasHint() const;

	virtual SyncTask *createSyncTask(QString mode,
									 const QDir &syncDir,
									 QString path,
									 QStringList extras,
									 bool isFirstUse,
									 QObject *parent) = 0;
	virtual SyncTask *createUndoSyncTask(QString mode,
										 const QDir &syncDir,
										 QString path,
										 QObject *parent) = 0;
};

class NotASymlinkException {};

#endif // SYNCHELPER_H
