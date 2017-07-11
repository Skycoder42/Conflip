#include "settingsdatabase.h"
#include "libconflip_global.h"

#include <QException>

SettingsEntry::SettingsEntry(QObject *parent) :
	QObject(parent),
	keyChain(),
	recursive(false)
{}

SettingsValue::SettingsValue(QObject *parent, const QVariant &data) :
	QObject(parent),
	value(data),
	children()
{}

SettingsObject::SettingsObject(QObject *parent) :
	QObject(parent),
	id(),
	type(),
	paths(),
	entries(),
	data(new SettingsValue(this))
{}

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
		SettingsObject* lo = _serializer->deserialize<SettingsObject*>(local);
		SettingsObject* ro = _serializer->deserialize<SettingsObject*>(remote);

		//overwrite own path only
		auto id = deviceId();
		auto mPath = lo->paths.value(id);
		if(!mPath.isNull())
			ro->paths.insert(id, mPath);

		// update entries to merge lists and prefer local
		for(auto loIt = lo->entries.begin(); loIt != lo->entries.end(); loIt++) {
			auto lobj = *loIt;

			auto conflict = false;
			for(auto roIt = ro->entries.begin(); roIt != ro->entries.end(); roIt++) {
				auto robj = *roIt;

				if(robj->keyChain == lobj->keyChain) {
					robj->recursive = lobj->recursive;
					conflict = true;
				}
			}

			if(!conflict) {
				lobj->setParent(ro);
				ro->entries.append(lobj);
			}
		}

		mergeValue(lo->data, ro->data);

		return _serializer->serialize(ro);
	} catch(QException &e) {
		qWarning() << "Failed to merge with exception:" << e.what();
		return local;
	}
}

void SettingsObjectMerger::mergeValue(SettingsValue *lo, SettingsValue *ro)
{
	//prefer the local value
	ro->value = lo->value;

	for(auto loIt = lo->children.begin(); loIt != lo->children.end(); loIt++) {
		auto conflict = false;
		for(auto roIt = ro->children.begin(); roIt != ro->children.end(); roIt++) {
			if(roIt.key() == loIt.key()) {
				mergeValue(loIt.value(), roIt.value());
				conflict = true;
			}
		}

		if(!conflict) {
			loIt.value()->setParent(ro);
			ro->children.insert(loIt.key(), loIt.value());
		}
	}
}
