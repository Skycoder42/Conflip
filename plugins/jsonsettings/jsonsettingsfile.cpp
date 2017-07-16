#include "jsonsettingsfile.h"
#include <settingsplugin.h>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>

JsonSettingsFile::JsonSettingsFile(const QString &fileName, bool isBinary, QObject *parent) :
	SettingsFile(parent),
	_fileName(fileName),
	_binary(isBinary),
	_root(),
	_watcher(nullptr)
{
	readFile();
}

QStringList JsonSettingsFile::childGroups(const QStringList &parentChain)
{
	auto obj = getObject(parentChain);
	QStringList keys;
	for(auto it = obj.constBegin(); it != obj.constEnd(); it++) {
		if(it.value().isObject())
			keys.append(it.key());
	}
	return keys;
}

QStringList JsonSettingsFile::childKeys(const QStringList &parentChain)
{
	auto obj = getObject(parentChain);
	QStringList keys;
	for(auto it = obj.constBegin(); it != obj.constEnd(); it++) {
		if(!it.value().isObject())
			keys.append(it.key());
	}
	return keys;
}

bool JsonSettingsFile::isKey(const QStringList &keyChain)
{
	return !valueImp(keyChain).isObject();
}

QVariant JsonSettingsFile::value(const QStringList &keyChain)
{
	return valueImp(keyChain).toVariant();
}

void JsonSettingsFile::setValue(const QStringList &keyChain, const QVariant &value)
{
	if(keyChain.isEmpty())
		return;
	else {
		auto baseChain = keyChain;
		auto key = baseChain.takeFirst();
		setValueImp(baseChain, _root[key], QJsonValue::fromVariant(value));
		writeFile();
	}
}

void JsonSettingsFile::autoBackup()
{
	if(!QFile::copy(_fileName, _fileName + QStringLiteral(".bkp"))) {
		qWarning() << "Unable to create backup for"
				   << _fileName;
	}
}

void JsonSettingsFile::watchChanges()
{
	if(tryReadFile()) {
		_watcher = new QFileSystemWatcher(this);
		connect(_watcher, &QFileSystemWatcher::fileChanged, this, [this](QString file){
			if(tryReadFile())
				emit settingsChanged();
			_watcher->addPath(file);
		});

		if(!_watcher->addPath(_fileName))
			qWarning() << "Failed to watch for changes on" << _fileName;
	}
}

void JsonSettingsFile::readFile()
{
	QFile file(_fileName);
	if(!file.open(QIODevice::ReadOnly | (_binary ? QIODevice::NotOpen : QIODevice::Text)))
		throw SettingsLoadException(file.errorString().toUtf8());

	QJsonDocument doc;
	if(_binary) {
		doc = QJsonDocument::fromBinaryData(file.readAll());
		if(doc.isNull())
			throw SettingsLoadException("Invalid file. Cannot be read as binary json");
	} else {
		QJsonParseError error;
		doc = QJsonDocument::fromJson(file.readAll(), &error);
		if(error.error != QJsonParseError::NoError)
			throw SettingsLoadException(error.errorString().toUtf8());
	}

	file.close();
	if(!doc.isObject())
		throw SettingsLoadException("Only json files with objects as root element are supported");
	_root = doc.object();
}

bool JsonSettingsFile::tryReadFile()
{
	try {
		readFile();
		return true;
	} catch (QException &e) {
		qCritical() << "Failed to reload settings file"
					<< _fileName
					<< "with error"
					<< e.what();
		return false;
	}
}

void JsonSettingsFile::writeFile()
{
	QFile file(_fileName);
	if(!file.open(QIODevice::WriteOnly | (_binary ? QIODevice::NotOpen : QIODevice::Text))) {
		qCritical() << "Failed to open json file for writing"
					<< _fileName
					<< "with error"
					<< file.errorString();
		return;
	}

	QJsonDocument doc(_root);
	if(_binary)
		file.write(doc.toBinaryData());
	else
		file.write(doc.toJson(QJsonDocument::Indented));
	file.close();
}

QJsonObject JsonSettingsFile::getObject(const QStringList &keyChain)
{
	auto current = _root;
	foreach (auto key, keyChain) {
		auto next = current.value(key);
		if(next.isObject())
			current = next.toObject();
		else
			return QJsonObject();
	}
	return current;
}

QJsonValue JsonSettingsFile::valueImp(const QStringList &keyChain)
{
	auto baseChain = keyChain;
	auto key = baseChain.takeLast();
	return getObject(baseChain).value(key);
}

void JsonSettingsFile::setValueImp(QStringList keyChain, QJsonValueRef parent, const QJsonValue &value)
{
	if(keyChain.isEmpty())
		parent = value;
	else {
		auto key = keyChain.takeFirst();
		auto obj = parent.toObject();
		setValueImp(keyChain, obj[key], value);
		parent = obj;
	}
}
