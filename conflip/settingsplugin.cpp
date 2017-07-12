#include "settingsplugin.h"

SettingsPlugin::SettingsPlugin(QObject *parent) :
	QObject(parent)
{}



SettingsLoadException::SettingsLoadException(const QByteArray &what) :
	QException(),
	_what(what)
{}

const char *SettingsLoadException::what() const noexcept
{
	return _what.constData();
}

void SettingsLoadException::raise() const
{
	throw *this;
}

QException *SettingsLoadException::clone() const
{
	return new SettingsLoadException(_what);
}
