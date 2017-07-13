#include "qsettingsfile.h"

#include <QFile>
#include <QDebug>

QSettingsFile::QSettingsFile(QSettings *settings, QObject *parent) :
	SettingsFile(parent),
	_settings(settings),
	_watcher(new QFileSystemWatcher(this))
{
	_settings->setParent(this);

	connect(_watcher, &QFileSystemWatcher::fileChanged, this, [this](QString file){
		emit settingsChanged();
		_watcher->addPath(file);
	});

	if(!_watcher->addPath(_settings->fileName()))
		qWarning() << "Failed to watch for changes on" << _settings->fileName();
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
