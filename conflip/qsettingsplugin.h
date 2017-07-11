#ifndef QSETTINGSPLUGIN_H
#define QSETTINGSPLUGIN_H

#include <QSettings>
#include "settingsplugin.h"

class LIBCONFLIP_EXPORT QSettingsPlugin : public SettingsPlugin
{
	Q_OBJECT

public:
	explicit QSettingsPlugin(QObject *parent = nullptr);

	virtual QSettings::Format registerFormat(const QString &type) = 0;

	SettingsFile *createSettings(const QString &path, const QString &type, QObject *parent) final;

private:
	QHash<QString, QSettings::Format> _formats;
};

#endif // QSETTINGSPLUGIN_H
