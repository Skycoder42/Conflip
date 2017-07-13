#ifndef QSETTINGSFILE_H
#define QSETTINGSFILE_H

#include "settingsfile.h"
#include <QFileSystemWatcher>
#include <QSettings>

class QSettingsFile : public SettingsFile
{
	Q_OBJECT

public:
	explicit QSettingsFile(QSettings *settings, QObject *parent = nullptr);

	bool hasChildren(const QStringList &parentChain) override;
	QStringList childGroups(const QStringList &parentChain) override;
	QStringList childKeys(const QStringList &parentChain) override;
	QVariant value(const QStringList &keyChain) override;
	void setValue(const QStringList &keyChain, const QVariant &value) override;

	void autoBackup() override;
	void watchChanges() override;

private:
	QSettings *_settings;
	QFileSystemWatcher *_watcher;
};

#endif // QSETTINGSFILE_H
