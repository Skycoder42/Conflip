#ifndef INISYNCHELPER_H
#define INISYNCHELPER_H

#include <QObject>
#include <synchelper.h>

class IniSyncHelper;
class IniSyncTask : public SyncTask
{
	Q_OBJECT
public:
	// sync
	IniSyncTask(const IniSyncHelper *helper,
				QString &&mode,
				const QDir &syncDir,
				QString &&path,
				QStringList &&extras,
				bool isFirstUse,
				QObject *parent);
	// remove
	IniSyncTask(const IniSyncHelper *helper,
				QString &&mode,
				const QDir &syncDir,
				QString &&path,
				QObject *parent);

protected:
	void performSync() override;

private:
	using KeyInfo = std::pair<QList<QByteArrayList>, QList<QByteArrayList>>;
	using IniGroupMapping = QMap<QByteArray, QByteArray>;
	using IniEntryMapping = QMap<QByteArray, IniGroupMapping>;

	IniEntryMapping readSyncMapping(const QFileInfo &sync);
	void writeMapping(QIODevice *device, const IniEntryMapping &mapping, bool firstLine, bool &needSave, bool log = true);
	void writeMapping(const QFileInfo &file, const IniEntryMapping &mapping);
	bool shouldSync(const QByteArray &group, const QByteArray &key, const KeyInfo &extras);
	bool startsWith(const QByteArrayList &key, const QByteArrayList &subList);

	QString logKey(const QByteArray &cGroup, const QByteArray &key) const;
};

class IniSyncHelper : public SyncHelper
{
	Q_OBJECT

public:
	static const QString ModeIni;

	explicit IniSyncHelper(QObject *parent = nullptr);

	bool pathIsPattern(const QString &mode) const override;
	bool canSyncDirs(const QString &mode) const override;
	ExtrasHint extrasHint() const override;

	SyncTask *createSyncTask(QString mode, const QDir &syncDir, QString path, QStringList extras, bool isFirstUse, QObject *parent) override;
	SyncTask *createUndoSyncTask(QString mode, const QDir &syncDir, QString path, QObject *parent) override;

private:
	using KeyInfo = std::pair<QList<QByteArrayList>, QList<QByteArrayList>>;
	using IniGroupMapping = QMap<QByteArray, QByteArray>;
	using IniEntryMapping = QMap<QByteArray, IniGroupMapping>;

	IniEntryMapping createMapping(const QFileInfo &file) const;
	void writeMapping(QIODevice *device, const IniEntryMapping &mapping, bool firstLine, bool &needSave, const QString &logStr = {}) const;
	void writeMapping(const QFileInfo &file, const IniEntryMapping &mapping);
	bool shouldSync(const QByteArray &group, const QByteArray &key, const KeyInfo &extras) const;
	bool startsWith(const QByteArrayList &key, const QByteArrayList &subList) const;

	void log(const QFileInfo &file, const char *msg, bool dbg = false) const;
	void log(const QFileInfo &file, const char *msg, const QByteArray &cGroup, const QByteArray &key, bool dbg = false) const;
};

#endif // INISYNCHELPER_H
