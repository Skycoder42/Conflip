#ifndef XMLSYNCHELPER_H
#define XMLSYNCHELPER_H

#include <synchelper.h>
#include <QDomDocument>
#include <QMutex>

class XmlSyncHelper;
class XmlSyncTask : public SyncTask
{
	Q_OBJECT

public:
	// sync
	XmlSyncTask(const XmlSyncHelper *helper,
				QString &&mode,
				const QDir &syncDir,
				QString &&path,
				QStringList &&extras,
				bool isFirstUse,
				QObject *parent);
	// remove
	XmlSyncTask(const XmlSyncHelper *helper,
				QString &&mode,
				const QDir &syncDir,
				QString &&path,
				QObject *parent);

protected:
	void performSync() override;

private:
	static QMutex _XmlMutex;

	void updateText(const QDomElement& srcElement, const QDomElement& syncElement,
					bool srcExists, bool syncExists,
					bool srcIsNewer,
					bool &srcNeedsUpdate, bool &syncNeedsUpdate,
					const QString &key);
	void updateAttributes(const QDomElement& srcElement, const QDomElement& syncElement,
						  bool srcIsNewer,
						  bool &srcNeedsUpdate, bool &syncNeedsUpdate,
						  const QString &key);
	void updateAttribute(QDomElement srcElement, QDomElement syncElement,
						 QDomAttr srcAttr, QDomAttr syncAttr,
						 bool srcIsNewer,
						 bool &srcNeedsUpdate, bool &syncNeedsUpdate,
						 const QString &key);

	QDomDocument loadDocument(const QFileInfo &file) const;
	void saveDocument(const QFileInfo &file, const QDomDocument& document);

	QDomDocument createDoc(const QDomDocument& other) const;
	QDomElement cd(QDomElement current, const QString &tag, bool &exists) const;
	QString getText(const QDomElement& element) const;
	void writeText(QDomElement element, const QString &text) const;
	void removeAttribs(QDomElement node) const;
};

class XmlSyncHelper : public SyncHelper
{
	Q_OBJECT

public:
	static const QString ModeXml;

	explicit XmlSyncHelper(QObject *parent = nullptr);

	bool pathIsPattern(const QString &mode) const override;
	bool canSyncDirs(const QString &mode) const override;
	ExtrasHint extrasHint() const override;

	SyncTask *createSyncTask(QString mode, const QDir &syncDir, QString path, QStringList extras, bool isFirstUse, QObject *parent) override;
	SyncTask *createUndoSyncTask(QString mode, const QDir &syncDir, QString path, QObject *parent) override;
};

#endif // XMLSYNCHELPER_H
