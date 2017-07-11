#ifndef SETTINGSPLUGIN_H
#define SETTINGSPLUGIN_H

#include "libconflip_global.h"

#include <QObject>

#define SettingsPlugin_iid "de.skycoder42.conflip.SettingsPlugin"

class LIBCONFLIP_EXPORT SettingsPlugin : public QObject
{
	Q_OBJECT

public:
	SettingsPlugin(QObject *parent = nullptr);

	virtual int baum() const = 0;
};

#endif // SETTINGSPLUGIN_H
