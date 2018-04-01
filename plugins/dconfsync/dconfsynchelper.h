#ifndef DCONFSYNCHELPER_H
#define DCONFSYNCHELPER_H

#include <QDateTime>
#include <QJsonSerializer>
#include <QObject>
#include <QSettings>
#include <synchelper.h>

class DConfEntry
{
	Q_GADGET

	Q_PROPERTY(QString type READ getType WRITE setType)
	Q_PROPERTY(QByteArray data MEMBER data)
	Q_PROPERTY(QDateTime lastModified MEMBER lastModified)

public:
	QByteArray type;
	QByteArray data;
	QDateTime lastModified;

private:
	QString getType() const;
	void setType(const QString &value);
};

class DConfSyncHelper : public SyncHelper
{
	Q_OBJECT

public:
	static const QString ModeDConf;

	explicit DConfSyncHelper(QObject *parent = nullptr);

	bool pathIsPattern(const QString &mode) const override;
	void performSync(const QString &path, const QString &mode, const QStringList &extras, bool isFirstUse) override;

private:
	using DConfMap = QMap<QByteArray, DConfEntry>;
	QJsonSerializer *_serializer;

	QSharedPointer<QSettings> loadSettings(const QString &path);
	DConfMap readSyncConf(const QFileInfo &file) const;
	void writeSyncConf(const QFileInfo &file, const DConfMap &map);

	void log(const QString &path, const char *msg, bool dbg = false) const;
	void log(const QString &path, const char *msg, const QByteArray &key, bool dbg = false) const;
};

#endif // DCONFSYNCHELPER_H
