#include "qsettingsfile.h"

#include <QFile>
#include <QDebug>
#include <QFileInfo>

QSettingsFile::QSettingsFile(QSettings *settings, QObject *parent) :
	SettingsFile(parent),
	_settings(settings),
	_watcher(nullptr)
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

void QSettingsFile::autoBackup()
{
	if(!QFile::copy(_settings->fileName(), _settings->fileName() + QStringLiteral(".bkp"))) {
		qWarning() << "Unable to create backup for"
				   << _settings->fileName();
	}
}

void QSettingsFile::watchChanges()
{
	_settings->sync();

	_watcher = new QFileSystemWatcher(this);
	connect(_watcher, &QFileSystemWatcher::fileChanged, this, [this](QString file){
		_settings->sync();
		emit settingsChanged();
		_watcher->addPath(file);
	});

	if(!_watcher->addPath(_settings->fileName()))
		qWarning() << "Failed to watch for changes on" << _settings->fileName();
}
