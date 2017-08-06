#ifndef FILESETTINGSFILE_H
#define FILESETTINGSFILE_H

#include <settingsfile.h>

class FileSettingsFile : public FileBasedSettingsFile
{
	Q_OBJECT

public:
	explicit FileSettingsFile(const QString &_path, QObject *parent = nullptr);

	QStringList childGroups(const QStringList &parentChain) override;
	QStringList childKeys(const QStringList &parentChain) override;
	bool hasChildren(const QStringList &parentChain) override;
	bool isKey(const QStringList &keyChain) override;
	QVariant value(const QStringList &keyChain) override;
	void setValue(const QStringList &keyChain, const QVariant &value) override;

protected:
	QString filePath() override;
	void readFile() override;

private:
	static const QStringList DataChain;

	QString _path;
	QByteArray _data;
};

#endif // FILESETTINGSFILE_H
