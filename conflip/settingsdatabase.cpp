#include "settingsdatabase.h"
#include "libconflip_global.h"

#include <QDataStream>
#include <QException>

SettingsObject::SettingsObject() :
	id(),
	type(),
	paths(),
	syncAll(false),
	entries()
{}

bool SettingsObject::isValid() const
{
	return !id.isNull();
}

QString SettingsObject::devicePath() const
{
	auto p = paths.value(deviceId());
	if(p.isNull() && !paths.isEmpty())
		return paths.values().first();
	else
		return p;
}

bool SettingsObject::operator ==(const SettingsObject &other) const
{
	return id == other.id;
}

bool SettingsObject::operator !=(const SettingsObject &other) const
{
	return id != other.id;
}

QMap<QString, QString> SettingsObject::getPaths() const
{
	QMap<QString, QString> res;
	for(auto it = paths.begin(); it != paths.end(); it++)
		res.insert(it.key().toString(), it.value());
	return res;
}

void SettingsObject::setPaths(const QMap<QString, QString> &value)
{
	paths.clear();
	for(auto it = value.begin(); it != value.end(); it++)
		paths.insert(it.key(), it.value());
}

SettingsEntry::SettingsEntry() :
	keyChain(),
	recursive(false)
{}

bool SettingsEntry::operator ==(const SettingsEntry &other) const
{
	return keyChain == other.keyChain;
}

bool SettingsEntry::operator !=(const SettingsEntry &other) const
{
	return keyChain != other.keyChain;
}

SettingsValue::SettingsValue() :
	objectId(),
	keyChain(),
	entryChain(),
	value()
{}

QString SettingsValue::id() const
{
	return objectId.toString() +
			QLatin1Char('-') +
			QUuid::createUuidV5(objectId, keyChain.join(QLatin1Char('/'))).toString();
}

bool SettingsValue::operator ==(const SettingsValue &other) const
{
	return id() == other.id();
}

bool SettingsValue::operator !=(const SettingsValue &other) const
{
	return id() != other.id();
}

QString SettingsValue::getValue() const
{
	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
	stream << value;
	return QString::fromUtf8(buffer.toBase64());
}

void SettingsValue::setValue(const QString &value)
{
	auto buffer = QByteArray::fromBase64(value.toUtf8());
	QDataStream stream(buffer);
	stream >> this->value;
}

SettingsObjectMerger::SettingsObjectMerger(QObject *parent) :
	DataMerger(parent),
	_serializer(new QJsonSerializer(this))
{
	setSyncPolicy(PreferDeleted);
	setMergePolicy(Merge);
}

QJsonObject SettingsObjectMerger::merge(QJsonObject local, QJsonObject remote)
{
	//assume SettingsObject
	try {
		if(local.contains(QStringLiteral("type"))) { //SettingsObject
			auto lo = _serializer->deserialize<SettingsObject>(local);
			auto ro = _serializer->deserialize<SettingsObject>(remote);

			//overwrite own path only
			auto id = deviceId();
			ro.paths.insert(id, lo.devicePath());

			// update entries to merge lists and prefer local
			foreach(auto entry, lo.entries) {
				auto roIndex = ro.entries.indexOf(entry);
				if(roIndex != -1) {
					auto &roEntry = ro.entries[roIndex];
					// prefer recursive
					if(entry.recursive != roEntry.recursive)
						roEntry.recursive = true;
				} else
					ro.entries.append(entry);
			}

			return _serializer->serialize(ro);
		} else if(local.contains(QStringLiteral("objectId"))) //SettingsValue
			return remote;// TODO prefer local or prefer remote state ???
	} catch(QException &e) {
		qWarning() << "Failed to merge with exception:" << e.what();
	}

	return local;
}
