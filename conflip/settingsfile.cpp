#include "settingsfile.h"
#include <QFile>
#include <QFileSystemWatcher>
#include <QDebug>
#include <QException>

SettingsFile::SettingsFile(QObject *parent) :
	QObject(parent)
{}

bool SettingsFile::hasChildren(const QStringList &parentChain)
{
	return !childGroups(parentChain).isEmpty() ||
			!childKeys(parentChain).isEmpty();
}

bool SettingsFile::isKey(const QStringList &keyChain)
{
	if(keyChain.isEmpty())
		return false;
	auto parentChain = keyChain;
	auto key = parentChain.takeLast();
	return childKeys(parentChain).contains(key);
}



FileBasedSettingsFile::FileBasedSettingsFile(QObject *parent) :
	SettingsFile(parent)
{}

void FileBasedSettingsFile::autoBackup()
{
	auto path = filePath();
	auto bkp = path + QStringLiteral(".bkp");
	if(QFile::exists(bkp))
		qInfo() << "backup already exists for" << path;
	else if(!QFile::copy(path, bkp)) {
		qWarning() << "Unable to create backup for"
				   << path;
	}
}

void FileBasedSettingsFile::watchChanges()
{
	if(tryReadFile()) {
		_watcher = new QFileSystemWatcher(this);
		connect(_watcher, &QFileSystemWatcher::fileChanged, this, [this](QString file){
			if(tryReadFile())
				emit settingsChanged();
			_watcher->addPath(file);
		});

		auto path = filePath();
		if(!_watcher->addPath(path))
			qWarning() << "Failed to watch for changes on" << path;
	}
}

bool FileBasedSettingsFile::tryReadFile()
{
	try {
		readFile();
		return true;
	} catch (QException &e) {
		qCritical() << "Failed to reload settings file"
					<< filePath()
					<< "with error"
					<< e.what();
		return false;
	}
}
