#include "settingsdatabase.h"
#include "libconflip_global.h"

#include <QException>

bool SettingsEntry::operator ==(const SettingsEntry &other) const
{
	return keyChain == other.keyChain &&
			recursive == other.recursive;
}

bool SettingsEntry::operator !=(const SettingsEntry &other) const
{
	return keyChain != other.keyChain ||
			recursive != other.recursive;
}

SettingsValue::SettingsValue(const QVariant &data) :
	value(data),
	_children()
{}

QHash<QString, SettingsValue> SettingsValue::children() const
{
	QHash<QString, SettingsValue> res;
	for(auto it = _children.constBegin(); it != _children.constEnd(); it++)
		res.insert(it.key(), *it.value());
	return res;
}

void SettingsValue::setChildren(const QHash<QString, SettingsValue> &value)
{
	_children.clear();
	for(auto it = value.constBegin(); it != value.constEnd(); it++)
		_children.insert(it.key(), QSharedPointer<SettingsValue>::create(it.value()));
}

bool SettingsValue::operator ==(const SettingsValue &other) const
{
	return value == other.value &&
			children() == other.children();
}

bool SettingsValue::operator !=(const SettingsValue &other) const
{
	return value != other.value ||
			children() != other.children();
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
		SettingsObject lo = _serializer->deserialize<SettingsObject>(local);
		SettingsObject ro = _serializer->deserialize<SettingsObject>(remote);

		//overwrite own path only
		auto id = deviceId();
		ro.paths.insert(id, lo.paths.value(id));

		// update entries to merge lists and prefer local
		for(auto loIt = lo.entries.begin(); loIt != lo.entries.end(); loIt++) {
			auto conflict = false;
			for(auto roIt = ro.entries.begin(); roIt != ro.entries.end(); roIt++) {
				if(loIt->keyChain == roIt->keyChain) {
					roIt->recursive = loIt->recursive;
					conflict = true;
				}
			}
			if(!conflict)
				ro.entries.append(*loIt);
		}

		ro.data = mergeValue(lo.data, ro.data);

		return _serializer->serialize(ro);
	} catch(QException &e) {
		qWarning() << "Failed to merge with exception:" << e.what();
		return local;
	}
}

SettingsValue SettingsObjectMerger::mergeValue(SettingsValue lo, SettingsValue ro)
{
	//prefer the local value
	ro.value = lo.value;

	for(auto loIt = lo._children.begin(); loIt != lo._children.end(); loIt++) {
		auto conflict = false;
		for(auto roIt = ro._children.begin(); roIt != ro._children.end(); roIt++) {
			if(loIt.key() == roIt.key()) {
				auto res = mergeValue(*loIt.value(), *roIt.value());
				*roIt = QSharedPointer<SettingsValue>::create(res);
				conflict = true;
			}
		}
		if(!conflict)
			ro._children.insert(loIt.key(), loIt.value());
	}

	return ro;
}
