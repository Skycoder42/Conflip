#ifndef SETTINGSPLUGIN_H
#define SETTINGSPLUGIN_H

#include "libconflip_global.h"
#include <QObject>
#include <QException>
#include "settingsfile.h"

#define SettingsPlugin_iid "de.skycoder42.conflip.SettingsPlugin"

class LIBCONFLIP_EXPORT SettingsLoadException : public QException
{
public:
	SettingsLoadException(const QByteArray &what);

	const char *what() const noexcept override;
	void raise() const override;
	QException *clone() const override;

private:
	const QByteArray _what;
};

class LIBCONFLIP_EXPORT SettingsPlugin : public QObject
{
	Q_OBJECT

public:
	SettingsPlugin(QObject *parent = nullptr);

	virtual SettingsFile *createSettings(const QString &path, const QString &type, QObject *parent = nullptr) = 0;
	virtual QString displayName(const QString &type) const = 0;
};

#endif // SETTINGSPLUGIN_H
