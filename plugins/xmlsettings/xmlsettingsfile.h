#ifndef XMLSETTINGSFILE_H
#define XMLSETTINGSFILE_H

#include <QFileSystemWatcher>
#include <QDomDocument>
#include <settingsfile.h>

class XmlSettingsFile : public FileBasedSettingsFile
{
	Q_OBJECT
public:
	explicit XmlSettingsFile(const QString &fileName, QObject *parent = nullptr);

	QStringList childGroups(const QStringList &parentChain) override;
	QStringList childKeys(const QStringList &parentChain) override;
	QVariant value(const QStringList &keyChain) override;
	void setValue(const QStringList &keyChain, const QVariant &value) override;

private:
	static const QString DefaultKey;

	QString _fileName;
	QDomDocument _doc;

	QString filePath() const override;
	void readFile() override;
	void writeFile();

	QDomNode getNode(const QStringList &keyChain);
};

#endif // XMLSETTINGSFILE_H
