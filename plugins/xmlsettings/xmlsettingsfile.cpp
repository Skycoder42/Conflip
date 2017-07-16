#include "xmlsettingsfile.h"
#include <QDebug>
#include <QFile>
#include <settingsplugin.h>

XmlSettingsFile::XmlSettingsFile(const QString &fileName, QObject *parent) :
	SettingsFile(parent),
	_fileName(),
	_doc(),
	_watcher(nullptr)
{}

QStringList XmlSettingsFile::childGroups(const QStringList &parentChain)
{
	QHash<QString, int> groups;

	auto child = getElement(parentChain).firstChildElement();
	while(!child.isNull()) {
		auto grandChild = child.firstChildElement();
		if(!grandChild.isNull())
			groups[child.tagName()]++;
		child = child.nextSiblingElement();
	}

	for(auto it = groups.begin(); it != groups.end();) {
		if(it.value() > 1)
			it = groups.erase(it);
		else
			it++;
	}
	return groups.keys();
}

QStringList XmlSettingsFile::childKeys(const QStringList &parentChain)
{
	QSet<QString> children;
	QSet<QString> skipped;

	auto element = getElement(parentChain);

	auto attribs = element.attributes();
	for(auto i = 0; i < attribs.size(); i++)
		children.insert(attribs.item(i).toAttr().name());

	auto child = element.firstChildElement();
	while(!child.isNull()) {
		if(skipped.contains(child.tagName()))//skipped == group with that name exists -> now becomes "array"
			children.insert(child.tagName());
		else if(!child.firstChildElement().isNull())//child has children -> group -> skip
			skipped.insert(child.tagName());

		child = child.nextSiblingElement();
	}

	return children.toList();
}

QVariant XmlSettingsFile::value(const QStringList &keyChain)
{
	auto baseChain = keyChain;
	auto key = baseChain.takeLast();
	auto element = getElement(baseChain);

	//check if array
	auto keyElement = element.firstChildElement(key);
	if(!keyElement.isNull()) { //is array
		QStringList serData;
		do {
			QString data;
			QTextStream stream(&data, QIODevice::ReadOnly);
			keyElement.save(stream, 0);
			stream.flush();
			serData.append(data);

			keyElement = keyElement.nextSiblingElement(key);
		} while(!keyElement.isNull());
		return serData;
	} else // is not
		return element.attribute(key);
}

void XmlSettingsFile::setValue(const QStringList &keyChain, const QVariant &value)
{
	auto baseChain = keyChain;
	auto key = baseChain.takeLast();
	auto element = getElement(baseChain);

	if(value.type() == QVariant::StringList) {//is array
		//delete all old elements
		auto remElem = element.elementsByTagName(key);
		for(auto i = 0; i < remElem.size(); i++)
			element.removeChild(remElem.at(i));

		//re-add new children
		foreach(auto data, value.toStringList()) {
			QDomDocument tDoc;
			if(tDoc.setContent(data))
				element.appendChild(tDoc.firstChildElement());
		}
	} else //is value
		element.setAttribute(key, value.toString());
}

void XmlSettingsFile::autoBackup()
{
	if(!QFile::copy(_fileName, _fileName + QStringLiteral(".bkp"))) {
		qWarning() << "Unable to create backup for"
				   << _fileName;
	}
}

void XmlSettingsFile::watchChanges()
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

void XmlSettingsFile::readFile()
{
	QFile file(_fileName);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		throw SettingsLoadException(file.errorString().toUtf8());

	QString error;
	if(!_doc.setContent(&file, &error))
		throw SettingsLoadException(error.toUtf8());

	file.close();
}

bool XmlSettingsFile::tryReadFile()
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

void XmlSettingsFile::writeFile()
{
	QFile file(_fileName);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qCritical() << "Failed to open json file for writing"
					<< _fileName
					<< "with error"
					<< file.errorString();
		return;
	}

	file.write(_doc.toByteArray(4));
	file.close();
}

QDomElement XmlSettingsFile::getElement(const QStringList &keyChain)
{
	if(keyChain.isEmpty())
		return QDomElement();
	else {
		QDomNode current = _doc;
		foreach(auto key, keyChain) {
			current = current.firstChildElement(key);
			if(current.isNull())
				return QDomElement();
		}
		return current.toElement();
	}
}
