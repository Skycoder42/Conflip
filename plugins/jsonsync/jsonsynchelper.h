#ifndef JSONSYNCHELPER_H
#define JSONSYNCHELPER_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <synchelper.h>

class JsonSyncHelper;
class JsonSyncTask : public SyncTask
{
	Q_OBJECT

public:
	// sync
	JsonSyncTask(const JsonSyncHelper *helper,
				 QString &&mode,
				 const QDir &syncDir,
				 QString &&path,
				 QStringList &&extras,
				 bool isFirstUse,
				 QObject *parent);
	// remove
	JsonSyncTask(const JsonSyncHelper *helper,
				 QString &&mode,
				 const QDir &syncDir,
				 QString &&path,
				 QObject *parent);
protected:
	void performSync() override;

private:
	const bool _isBinary;
	QList<QStringList> _keyChains;
	bool _srcIsNewer = false;
	bool _srcNeedsUpdate = false;
	bool _syncNeedsUpdate = false;

	void performArraySync(QJsonArray &srcArray,
						  const QJsonArray &syncArray,
						  QJsonArray &resArray);
	void performObjSync(QJsonObject &srcObj,
						const QJsonObject &syncObj,
						QJsonObject &resObj);

	void traverse(QJsonObject &srcObj,
				  const QJsonObject &syncObj,
				  QJsonObject &resObj,
				  const QStringList &keyChain,
				  int keyIndex);
	QJsonObject getChild(const QJsonObject &srcObj,
						 const QString &key,
						 const QStringList &keyChain,
						 int keyIndex, const QByteArray &target);

	void syncElement(QJsonObject &srcObj,
					 const QJsonObject &syncObj,
					 QJsonObject &resObj,
					 const QString &key,
					 const QString &logKey);

	QJsonDocument readFile(const QString &path, const QByteArray &target);
	void writeFile(const QString &path, const QJsonDocument &doc, const QByteArray &target);
};

class JsonSyncHelper : public SyncHelper
{
	Q_OBJECT

public:
	static const QString ModeJson;
	static const QString ModeQbjs;

	explicit JsonSyncHelper(QObject *parent = nullptr);

	bool pathIsPattern(const QString &mode) const override;
	bool canSyncDirs(const QString &mode) const override;
	ExtrasHint extrasHint() const override;

	SyncTask *createSyncTask(QString mode, const QDir &syncDir, QString path, QStringList extras, bool isFirstUse, QObject *parent) override;
	SyncTask *createUndoSyncTask(QString mode, const QDir &syncDir, QString path, QObject *parent) override;

private:
	bool _isBinary;

	void performArraySync(QJsonArray &srcArray,
						  const QJsonArray &syncArray,
						  QJsonArray &resArray,
						  const QList<QStringList> &keyChains,
						  bool srcIsNewer,
						  bool &srcNeedsUpdate,
						  bool &syncNeedsUpdate,
						  const QString &logPath);
	void performObjSync(QJsonObject &srcObj,
						const QJsonObject &syncObj,
						QJsonObject &resObj,
						const QList<QStringList> &keyChains,
						bool srcIsNewer,
						bool &srcNeedsUpdate,
						bool &syncNeedsUpdate,
						const QString &logPath);

	void traverse(QJsonObject &srcObj,
				  const QJsonObject &syncObj,
				  QJsonObject &resObj,
				  const QStringList &keyChain,
				  int keyIndex,
				  bool srcIsNewer,
				  bool &srcNeedsUpdate,
				  bool &syncNeedsUpdate,
				  const QString &logPath);
	QJsonObject getChild(const QJsonObject &srcObj,
						 const QString &key,
						 const QStringList &keyChain,
						 int keyIndex, const QByteArray &target);

	void syncElement(QJsonObject &srcObj,
					 const QJsonObject &syncObj,
					 QJsonObject &resObj,
					 const QString &key,
					 bool srcIsNewer,
					 bool &srcNeedsUpdate,
					 bool &syncNeedsUpdate,
					 const QString &logPath,
					 const QByteArray &logKey);

	QJsonDocument readFile(const QString &path, const QByteArray &target);
	void writeFile(const QString &path, const QJsonDocument &doc, const QByteArray &target);

	void log(const QString &path, const char *msg, bool dbg = false) const;
	void log(const QString &path, const char *msg, const QByteArray &key, bool dbg = false) const;
};

#endif // JSONSYNCHELPER_H
