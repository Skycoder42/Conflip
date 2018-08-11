#ifndef XMLSYNCHELPER_H
#define XMLSYNCHELPER_H

#include <synchelper.h>
#include <QDomDocument>

class XmlSyncHelper : public SyncHelper
{
	Q_OBJECT

public:
	static const QString ModeXml;

	explicit XmlSyncHelper(QObject *parent = nullptr);

	QString syncPrefix() const override;
	bool pathIsPattern(const QString &mode) const override;
	bool canSyncDirs(const QString &mode) const override;
	void performSync(const QString &path, const QString &mode, const QStringList &extras, bool isFirstUse) override;
	void undoSync(const QString &path, const QString &mode) override;
	ExtrasHint extrasHint() const override;

private:
	void updateText(const QDomElement& srcElement, const QDomElement& syncElement,
					bool srcExists, bool syncExists,
					bool srcIsNewer,
					bool &srcNeedsUpdate, bool &syncNeedsUpdate,
					const QString &key,
					const QFileInfo &srcInfo);
	void updateAttributes(const QDomElement& srcElement, const QDomElement& syncElement,
						  bool srcIsNewer,
						  bool &srcNeedsUpdate, bool &syncNeedsUpdate,
						  const QString &key,
						  const QFileInfo &srcInfo);
	void updateAttribute(QDomElement srcElement, QDomElement syncElement,
						 QDomAttr srcAttr, QDomAttr syncAttr,
						 bool srcIsNewer,
						 bool &srcNeedsUpdate, bool &syncNeedsUpdate,
						 const QString &key,
						 const QFileInfo &srcInfo);

	QDomDocument loadDocument(const QFileInfo &file) const;
	void saveDocument(const QFileInfo &file, const QDomDocument& document);

	void setupRootElements(QDomDocument &srcDoc, QDomDocument &syncDoc) const;
	QDomDocument createDoc(const QDomDocument& other) const;
	QDomElement cd(QDomElement current, const QString &tag, bool &exists) const;
	QString getText(const QDomElement& element) const;
	void writeText(QDomElement element, const QString &text) const;
	void removeAttribs(QDomElement node) const;

	void log(const QFileInfo &file, const char *msg, bool dbg = false) const;
	void log(const QFileInfo &file, const char *msg, const QString &key, bool dbg = false) const;
};

#endif // XMLSYNCHELPER_H
