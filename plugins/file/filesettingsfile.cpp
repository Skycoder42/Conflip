#include "filesettingsfile.h"

#include <settingsplugin.h>
#include <QFile>
#include <QDebug>

const QStringList FileSettingsFile::DataChain(QStringLiteral("data"));

FileSettingsFile::FileSettingsFile(const QString &path, QObject *parent) :
	FileBasedSettingsFile(parent),
	_path(path),
	_data()
{
	readFile();
}

QStringList FileSettingsFile::childGroups(const QStringList &parentChain)
{
	Q_UNUSED(parentChain)
	return QStringList();
}

QStringList FileSettingsFile::childKeys(const QStringList &parentChain)
{
	if(parentChain.isEmpty())
		return DataChain;
	else
		return QStringList();
}

bool FileSettingsFile::hasChildren(const QStringList &parentChain)
{
	return parentChain.isEmpty();
}

bool FileSettingsFile::isKey(const QStringList &keyChain)
{
	Q_UNUSED(keyChain)
	return true;
}

QVariant FileSettingsFile::value(const QStringList &keyChain)
{
	if(keyChain == DataChain)
		return _data;
	else
		return QVariant();
}

void FileSettingsFile::setValue(const QStringList &keyChain, const QVariant &value)
{
	if(keyChain == DataChain) {
		_data = value.toByteArray();
		//write file
		QFile file(_path);
		if(file.open(QIODevice::WriteOnly)) {
			file.write(_data);
			file.close();
		} else {
			qCritical() << "Failed to open file for writing"
						<< _path
						<< "with error"
						<< file.errorString();
		}
	}
}

QString FileSettingsFile::filePath() const
{
	return _path;
}

void FileSettingsFile::readFile()
{
	QFile file(_path);
	if(!file.open(QIODevice::ReadOnly))
		throw SettingsLoadException(file.errorString().toUtf8());

	_data = file.readAll();
	file.close();
}
