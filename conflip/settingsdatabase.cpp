#include "settingsdatabase.h"
#include "libconflip_global.h"

#include <QException>

SettingsObject::SettingsObject() :
	id(),
	type(),
	paths(),
	syncAll(false),
	entries(),
	values()
{}

bool SettingsObject::isValid() const
{
	return !id.isNull();
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

QList<QUuid> SettingsObject::getEntries() const
{
	return entries.toList();
}

void SettingsObject::setEntries(const QList<QUuid> &value)
{
	entries = QSet<QUuid>::fromList(value);
}

QList<QUuid> SettingsObject::getValues() const
{
	return values.toList();
}

void SettingsObject::setValues(const QList<QUuid> &value)
{
	values = QSet<QUuid>::fromList(value);
}

SettingsEntry::SettingsEntry() :
	objectId(),
	keyChain(),
	recursive(false)
{}

QUuid SettingsEntry::id() const
{
	return QUuid::createUuidV5(objectId, keyChain.join(QLatin1Char('/')));
}

bool SettingsEntry::operator ==(const SettingsEntry &other) const
{
	return id() == other.id();
}

bool SettingsEntry::operator !=(const SettingsEntry &other) const
{
	return id() != other.id();
}

SettingsValue::SettingsValue() :
	objectId(),
	keyChain(),
	value()
{}

QUuid SettingsValue::id() const
{
	return QUuid::createUuidV5(objectId, keyChain.join(QLatin1Char('/')));
}

bool SettingsValue::operator ==(const SettingsValue &other) const
{
	return id() == other.id();
}

bool SettingsValue::operator !=(const SettingsValue &other) const
{
	return id() != other.id();
}

SettingsObjectMerger::SettingsObjectMerger(QObject *parent) :
	DataMerger(parent),
	_serializer(new QJsonSerializer(this))
{
	setMergePolicy(Merge);
}

QJsonObject SettingsObjectMerger::merge(QJsonObject local, QJsonObject remote)
{
	//assume SettingsObject
	try {
		if(local.contains(QStringLiteral("type"))) {
			auto lo = _serializer->deserialize<SettingsObject>(local);
			auto ro = _serializer->deserialize<SettingsObject>(remote);

			//overwrite own path only
			auto id = deviceId();
			ro.paths.insert(id, lo.paths.value(id));

			// update entries to merge lists and prefer local
			ro.entries.unite(lo.entries);
			ro.values.unite(lo.values);

			return _serializer->serialize(ro);
		} else if(local.contains(QStringLiteral("recursive"))) {
			auto lo = _serializer->deserialize<SettingsEntry>(local);
			auto ro = _serializer->deserialize<SettingsEntry>(remote);

			// prefer recursive
			if(lo.recursive != ro.recursive)
				ro.recursive = true;

			return _serializer->serialize(ro);
		}
	} catch(QException &e) {
		qWarning() << "Failed to merge with exception:" << e.what();
	}

	return local;
}
