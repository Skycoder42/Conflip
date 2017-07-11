#include "qsettingsfile.h"

QSettingsFile::QSettingsFile(const QString &fileName, QSettings::Format format, QObject *parent) :
	SettingsFile(parent),
	_settings(new QSettings(fileName, format, this))
{}

bool QSettingsFile::hasChildren(const QStringList &parentChain)
{
	if(!parentChain.isEmpty())
		_settings->beginGroup(parentChain.join(QLatin1Char('/')));
	auto hasChildren = !_settings->allKeys().isEmpty();
	if(!parentChain.isEmpty())
		_settings->endGroup();
	return hasChildren;
}

QStringList QSettingsFile::childGroups(const QStringList &parentChain)
{
	if(!parentChain.isEmpty())
		_settings->beginGroup(parentChain.join(QLatin1Char('/')));
	auto children = _settings->childGroups();
	if(!parentChain.isEmpty())
		_settings->endGroup();
	return children;
}

QStringList QSettingsFile::childKeys(const QStringList &parentChain)
{
	if(!parentChain.isEmpty())
		_settings->beginGroup(parentChain.join(QLatin1Char('/')));
	auto children = _settings->childKeys();
	if(!parentChain.isEmpty())
		_settings->endGroup();
	return children;
}

QVariant QSettingsFile::value(const QStringList &keyChain)
{
	return _settings->value(keyChain.join(QLatin1Char('/')));
}

void QSettingsFile::setValue(const QStringList &keyChain, const QVariant &value)
{
	_settings->setValue(keyChain.join(QLatin1Char('/')), value);
}
