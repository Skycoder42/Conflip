#include "xmlsettingsfile.h"
#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <settingsplugin.h>

const QString XmlSettingsFile::DefaultKey = QStringLiteral("(default)");

XmlSettingsFile::XmlSettingsFile(const QString &fileName, QObject *parent) :
	FileBasedSettingsFile(parent),
	_fileName(fileName),
	_doc()
{
	readFile();
}

QStringList XmlSettingsFile::childGroups(const QStringList &parentChain)
{
	QHash<QString, int> groups;

	auto child = getNode(parentChain).firstChildElement();
	while(!child.isNull()) {
		groups[child.tagName()]++;
		child = child.nextSiblingElement();
	}

	QStringList keys;
	for(auto it = groups.begin(); it != groups.end(); it++) {
		for(auto i = 0; i < it.value(); i++)
			keys.append(QStringLiteral("%1[%2]").arg(it.key()).arg(i));
	}
	return keys;
}

QStringList XmlSettingsFile::childKeys(const QStringList &parentChain)
{
	QStringList children;

	auto element = getNode(parentChain);

	auto attribs = element.attributes();
	for(auto i = 0; i < attribs.size(); i++)
		children.append(attribs.item(i).toAttr().name());

	if(element.isElement()) {
		auto rElement = element.toElement();
		if(!rElement.text().isEmpty() && rElement.firstChildElement().isNull()) //has text but no child elements
			children.append(DefaultKey);
	}

	return children;
}

QVariant XmlSettingsFile::value(const QStringList &keyChain)
{
	auto baseChain = keyChain;
	auto key = baseChain.takeLast();
	auto parentElement = getNode(baseChain).toElement();

	if(key == DefaultKey)
		return parentElement.text();
	else
		return parentElement.attribute(key);
}

void XmlSettingsFile::setValue(const QStringList &keyChain, const QVariant &value)
{
	auto baseChain = keyChain;
	auto key = baseChain.takeLast();
	auto parentElement = getNode(baseChain).toElement();

	if(key == DefaultKey) {
		auto nodes = parentElement.childNodes();
		for(auto i = 0; i < nodes.size(); i++) {
			auto node = nodes.at(i);
			if(node.isText()) {
				auto text = node.toText();
				text.setData(value.toString());
				break;
			}
		}
	} else
		parentElement.setAttribute(key, value.toString());
}

QString XmlSettingsFile::filePath() const
{
	return _fileName;
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

QDomNode XmlSettingsFile::getNode(const QStringList &keyChain)
{
	QDomNode current = _doc;

	static QRegularExpression regex(QStringLiteral(R"__(^([^\[]*)\[(\d+)\]$)__"),
									QRegularExpression::OptimizeOnFirstUsageOption);

	foreach(auto key, keyChain) {
		auto match = regex.match(key);
		if(match.hasMatch()) {
			auto name = match.captured(1);
			auto index = match.captured(2).toInt();

			current = current.firstChildElement(name);
			for(auto i = 0; i < index && !current.isNull(); i++)
				current = current.nextSiblingElement(name);
		} else
			current = current.firstChildElement(key);//TODO array!!!

		if(current.isNull())
			return QDomNode();
	}
	return current;
}
