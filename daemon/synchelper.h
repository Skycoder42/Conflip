#ifndef SYNCHELPER_H
#define SYNCHELPER_H

#include <QDir>
#include <QException>
#include <QObject>
#include "syncentry.h"

class SyncHelper
{
public:
	virtual ~SyncHelper();

	void setSyncDir(const QDir &dir);

	virtual void performSync(const QString &path, SyncEntry::PathMode mode, const QStringList &extras) = 0;

protected:
	QDir syncDir() const;
	bool isFirstUse(const QString &path) const;
	void markFirstUsed(const QString &path);

private:
	QDir _syncDir;
};

class SyncException : public QException
{
public:
	SyncException(QByteArray what);

	const char *what() const noexcept override;
	void raise() const override;
	QException *clone() const override;

private:
	const QByteArray _what;
};

#define SyncHelperIid "de.skycoder42.conflip.SyncHelper"
Q_DECLARE_INTERFACE(SyncHelper, SyncHelperIid)

#endif // SYNCHELPER_H
