#ifndef PATHSYNCHELPER_H
#define PATHSYNCHELPER_H

#include <QCache>
#include <QObject>
#include <QRegularExpression>
#include <synchelper.h>

class PathSyncHelper;
class PathSyncTask : public SyncTask
{
	Q_OBJECT

public:
	// sync
	PathSyncTask(const PathSyncHelper *helper,
				 QString &&mode,
				 const QDir &syncDir,
				 QString &&path,
				 QStringList &&extras,
				 bool isFirstUse,
				 QObject *parent);
	// remove
	PathSyncTask(const PathSyncHelper *helper,
				 QString &&mode,
				 const QDir &syncDir,
				 QString &&path,
				 QObject *parent);

protected:
	void performSync() override;
	void undoSync() override;

private:
	mutable QCache<QString, QRegularExpression> _regexCache;

	void syncAsSymlink(const QFileInfo &src, const QFileInfo &sync);
	void syncAsCopy(const QFileInfo &src, const QFileInfo &sync);
	void unlink(const QFileInfo &src, const QFileInfo &sync);

	void movePath(const QFileInfo &from, const QFileInfo &to, bool fromIsSrc);
	bool removePath(const QFileInfo &path);

	QByteArray hashFile(const QFileInfo &file, const QByteArray &target) const;
};

class PathSyncHelper : public SyncHelper
{
	Q_OBJECT

public:
	static const QString ModeSymlink;
	static const QString ModeCopy;

	explicit PathSyncHelper(QObject *parent = nullptr);

	QString syncPrefix(const QString &mode) const override;
	bool pathIsPattern(const QString &mode) const override;
	bool canSyncDirs(const QString &mode) const override;
	ExtrasHint extrasHint() const override;

	SyncTask *createSyncTask(QString mode, const QDir &syncDir, QString path, QStringList extras, bool isFirstUse, QObject *parent) override;
	SyncTask *createUndoSyncTask(QString mode, const QDir &syncDir, QString path, QObject *parent) override;
};

#endif // PATHSYNCHELPER_H
