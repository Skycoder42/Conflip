#ifndef JSONSETTINGSFILE_H
#define JSONSETTINGSFILE_H

#include <QFileSystemWatcher>
#include <QJsonObject>
#include <settingsfile.h>

class JsonSettingsFile : public SettingsFile
{
	Q_OBJECT

public:
	explicit JsonSettingsFile(const QString &fileName,
							  bool isBinary,
							  QObject *parent = nullptr);

	QStringList childGroups(const QStringList &parentChain) override;
	QStringList childKeys(const QStringList &parentChain) override;
	bool isKey(const QStringList &keyChain) override;
	QVariant value(const QStringList &keyChain) override;
	void setValue(const QStringList &keyChain, const QVariant &value) override;
	void autoBackup() override;
	void watchChanges() override;

private:
	QString _fileName;
	bool _binary;
	QJsonObject _root;
	QFileSystemWatcher *_watcher;

	void readFile();
	bool tryReadFile();
	void writeFile();

	QJsonObject getObject(const QStringList &keyChain);
	QJsonValue valueImp(const QStringList &keyChain);
	void setValueImp(QStringList keyChain, QJsonValueRef parent, const QJsonValue &value);
};

#endif // JSONSETTINGSFILE_H
