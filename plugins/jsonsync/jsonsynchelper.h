#ifndef JSONSYNCHELPER_H
#define JSONSYNCHELPER_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <synchelper.h>

class JsonSyncHelper : public SyncHelper
{
	Q_OBJECT

public:
	static const QString ModeJson;
	static const QString ModeQbjs;

	explicit JsonSyncHelper(bool isBinary, QObject *parent = nullptr);

	QString syncPrefix() const override;
	bool pathIsPattern(const QString &mode) const override;
	bool canSyncDirs(const QString &mode) const override;
	void performSync(const QString &path, const QString &mode, const QStringList &extras, bool isFirstUse) override;
	void undoSync(const QString &path, const QString &mode) override;
	ExtrasHint extrasHint() const override;

private:
	const bool _isBinary;

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
