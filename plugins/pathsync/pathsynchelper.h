#ifndef PATHSYNCHELPER_H
#define PATHSYNCHELPER_H

#include <QCache>
#include <QObject>
#include <QRegularExpression>
#include <synchelper.h>

class PathSyncHelper : public SyncHelper
{
	Q_OBJECT

public:
	static const QString ModeSymlink;
	static const QString ModeCopy;

	explicit PathSyncHelper(QObject *parent = nullptr);

	QString syncPrefix() const override;
	bool pathIsPattern(const QString &mode) const override;
	void performSync(const QString &path, const QString &mode, const QStringList &extras, bool isFirstUse) override;
	void undoSync(const QString &path, const QString &mode) override;

private:
	mutable QCache<QString, QRegularExpression> _regexCache;

	void syncAsSymlink(const QFileInfo &src, const QFileInfo &sync, bool isFirstUse);
	void syncAsCopy(const QFileInfo &src, const QFileInfo &sync, bool isFirstUse);
	void unlink(const QFileInfo &src, const QFileInfo &sync);

	QByteArray hashFile(const QFileInfo &file) const;
	void log(const QFileInfo &file, const char *msg, bool dbg = false) const;
};

#endif // PATHSYNCHELPER_H
