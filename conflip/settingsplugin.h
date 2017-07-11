#ifndef SETTINGSPLUGIN_H
#define SETTINGSPLUGIN_H

#include "libconflip_global.h"
#include <QObject>
#include "settingsfile.h"

#define SettingsPlugin_iid "de.skycoder42.conflip.SettingsPlugin"

class LIBCONFLIP_EXPORT SettingsPlugin : public QObject
{
	Q_OBJECT

public:
	SettingsPlugin(QObject *parent = nullptr);

	virtual SettingsFile *createSettings(const QString &path, const QString &type, QObject *parent = nullptr) = 0;
};

#endif // SETTINGSPLUGIN_H
