#ifndef SETTINGSFILE_H
#define SETTINGSFILE_H

#include "libconflip_global.h"
#include <QObject>
#include <QVariant>
class QFileSystemWatcher;

class LIBCONFLIP_EXPORT SettingsFile : public QObject
{
	Q_OBJECT

public:
	explicit SettingsFile(QObject *parent = nullptr);

	virtual QStringList childGroups(const QStringList &parentChain) = 0;
	virtual QStringList childKeys(const QStringList &parentChain) = 0;
	virtual bool hasChildren(const QStringList &parentChain);
	virtual bool isKey(const QStringList &keyChain);
	virtual QVariant value(const QStringList &keyChain) = 0;
	virtual void setValue(const QStringList &keyChain, const QVariant &value) = 0;

	virtual void autoBackup() = 0;
	virtual void watchChanges() = 0;

signals:
	void settingsChanged(const QStringList &keyChain = {}, const QVariant &value = {});
};

class LIBCONFLIP_EXPORT FileBasedSettingsFile : public SettingsFile
{
public:
	explicit FileBasedSettingsFile(QObject *parent = nullptr);

	void autoBackup() override;
	void watchChanges() override;

protected:
	virtual QString filePath() const = 0;
	virtual void readFile() = 0;//throws SettingsLoadException

private:
	QFileSystemWatcher *_watcher;

	bool tryReadFile();
};

#endif // SETTINGSFILE_H
