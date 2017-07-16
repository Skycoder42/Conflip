#ifndef XMLSETTINGSFILE_H
#define XMLSETTINGSFILE_H

#include <QFileSystemWatcher>
#include <QDomDocument>
#include <settingsfile.h>

class XmlSettingsFile : public SettingsFile
{
	Q_OBJECT
public:
	explicit XmlSettingsFile(const QString &fileName, QObject *parent = nullptr);

	QStringList childGroups(const QStringList &parentChain) override;
	QStringList childKeys(const QStringList &parentChain) override;
	QVariant value(const QStringList &keyChain) override;
	void setValue(const QStringList &keyChain, const QVariant &value) override;
	void autoBackup() override;
	void watchChanges() override;

private:
	QString _fileName;
	QDomDocument _doc;
	QFileSystemWatcher *_watcher;

	void readFile();
	bool tryReadFile();
	void writeFile();

	QDomElement getElement(const QStringList &keyChain);
};

#endif // XMLSETTINGSFILE_H
