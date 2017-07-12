#ifndef QSETTINGSFILE_H
#define QSETTINGSFILE_H

#include "settingsfile.h"
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

private:
	QSettings *_settings;
};

#endif // QSETTINGSFILE_H
