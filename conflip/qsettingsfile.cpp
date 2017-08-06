#include "qsettingsfile.h"

#include <QFile>
#include <QDebug>
#include <QFileInfo>

QSettingsFile::QSettingsFile(QSettings *settings, QObject *parent) :
	FileBasedSettingsFile(parent),
	_settings(settings)
{
	_settings->setParent(this);
}

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

QString QSettingsFile::filePath() const
{
	return _settings->fileName();
}

void QSettingsFile::readFile()
{
	_settings->sync();
}
