#include "jsonsynchelper.h"
#include <QDateTime>
#include <QSaveFile>
#include <QDebug>

const QString JsonSyncHelper::ModeJson = QStringLiteral("json");
const QString JsonSyncHelper::ModeQbjs = QStringLiteral("qbjs");

JsonSyncHelper::JsonSyncHelper(QObject *parent) :
	SyncHelper{parent}
{}

bool JsonSyncHelper::pathIsPattern(const QString &mode) const
{
	Q_UNUSED(mode)
	return true;
}

bool JsonSyncHelper::canSyncDirs(const QString &mode) const
{
	Q_UNUSED(mode)
	return false;
}

SyncHelper::ExtrasHint JsonSyncHelper::extrasHint() const
{
	return {
		true,
		tr("&Keys"),
		tr("<p>Enter the key-path you want to synchronize. The path is a chain of keys of properties that "
		   "are traversed into the object and the last in the chain is synchronized. The keys are seperated "
		   "by a '/'.</p>"
		   "<p>For example, the key \"group/key\" would enter the property named \"group\" in the root element "
		   "and there get the value of the \"key\" property and synchronize it.</p>")
	};
}

SyncTask *JsonSyncHelper::createSyncTask(QString mode, const QDir &syncDir, QString path, QStringList extras, bool isFirstUse, QObject *parent)
{
	return new JsonSyncTask {
		this,
		std::move(mode),
		syncDir,
		std::move(path),
		std::move(extras),
		isFirstUse,
		parent
	};
}

SyncTask *JsonSyncHelper::createUndoSyncTask(QString mode, const QDir &syncDir, QString path, QObject *parent)
{
	return new JsonSyncTask {
		this,
		std::move(mode),
		syncDir,
		std::move(path),
		parent
	};
}



JsonSyncTask::JsonSyncTask(const JsonSyncHelper *helper, QString &&mode, const QDir &syncDir, QString &&path, QStringList &&extras, bool isFirstUse, QObject *parent) :
	SyncTask{helper, std::move(mode), syncDir, std::move(path), std::move(extras), isFirstUse, parent},
	_isBinary{this->mode == JsonSyncHelper::ModeQbjs}
{}

JsonSyncTask::JsonSyncTask(const JsonSyncHelper *helper, QString &&mode, const QDir &syncDir, QString &&path, QObject *parent) :
	SyncTask{helper, std::move(mode), syncDir, std::move(path), parent},
	_isBinary{this->mode == JsonSyncHelper::ModeQbjs}
{}

void JsonSyncTask::performSync()
{
	auto srcInfo = srcPath();
	auto syncInfo = syncPath();

	// step 0: prepare extras
	_keyChains.reserve(extras.size());
	for(const auto &extra : extras) {
		_keyChains.append(extra.split(QLatin1Char('/'), QString::SkipEmptyParts));
		if(_keyChains.last().isEmpty())
			fatal("Empty key found in extras - that is not allowed");
	}

	// step 1: read both files, determine the mode and prepare it
	_srcIsNewer = isFirstUse ? false : srcInfo.lastModified() > syncInfo.lastModified();
	auto srcDoc = readFile(srcInfo.absoluteFilePath(), "src");
	const auto syncDoc = readFile(syncInfo.absoluteFilePath(), "sync");
	if((srcDoc.isObject() && syncDoc.isArray()) ||
	   (syncDoc.isObject() && srcDoc.isArray()))
		fatal("Cannot synchronize files where one is a json array and the other a json object");

	// step 2: call sync methods, as array or as obj
	QJsonDocument resDoc;
	if(srcDoc.isArray() || syncDoc.isArray()) {
		// overwrite which one is newer as empty arrays do not communicate that
		if(!srcDoc.isArray())
			_srcIsNewer = false;
		if(!syncDoc.isArray())
			_srcIsNewer = true;
		auto srcArray = srcDoc.array();
		const auto syncArray = syncDoc.array();
		QJsonArray resArray;
		performArraySync(srcArray, syncArray, resArray);
		srcDoc.setArray(srcArray);
		resDoc.setArray(resArray);
	} else {
		auto srcObj = srcDoc.object();
		const auto syncObj = syncDoc.object();
		QJsonObject resObj;
		performObjSync(srcObj, syncObj, resObj);
		srcDoc.setObject(srcObj);
		resDoc.setObject(resObj);
	}

	// step 3: write the files that need changes
	if(_srcNeedsUpdate)
		writeFile(srcInfo.absoluteFilePath(), srcDoc, "src");
	if(_syncNeedsUpdate)
		writeFile(syncInfo.absoluteFilePath(), resDoc, "sync");
}

void JsonSyncTask::performArraySync(QJsonArray &srcArray, const QJsonArray &syncArray, QJsonArray &resArray)
{
	Q_ASSERT(resArray.isEmpty());
	// step 1: trim src array to max size to handle cases where src is longer but not wanted, and also log that
	auto arrayMax = _srcIsNewer ? srcArray.size() : syncArray.size();
	if(srcArray.size() > arrayMax) {
		info() << "Removed" << (srcArray.size() - arrayMax) << "elements from src array";
		while(srcArray.size() > arrayMax)
			srcArray.removeLast();
		_srcNeedsUpdate = true;
	}
	if(syncArray.size() > arrayMax) {
		info() << "Removed" << (syncArray.size() - arrayMax) << "elements from sync array";
		_syncNeedsUpdate = true;
	}

	// step 2: go through all elements that are in both arrays and sync them
	auto i = 0;
	for(auto nMax = std::min(srcArray.size(), syncArray.size()); i < nMax; ++i) {
		info() << "Operating on array element" << i;
		if(!srcArray[i].isUndefined() &&
		   !srcArray[i].isObject())
			fatal("Invalid element in src array - Only synchronization of json arrays made up of json objects are supported");
		if(!syncArray[i].isUndefined() &&
		   !syncArray[i].isObject())
			fatal("Invalid element in sync array - Only synchronization of json arrays made up of json objects are supported");
		QJsonObject srcObj = srcArray[i].toObject();
		QJsonObject resObj;
		performObjSync(srcObj, syncArray[i].toObject(), resObj);
		srcArray[i] = srcObj;
		resArray.append(resObj);
	}

	// step 3: sync the remaining elements that only exist in 1 list (only happens if wanted is the longer array)
	if(_srcIsNewer) { //case: src has more elements than sync and is needed
		for(; i < arrayMax; ++i) {
			info() << "Adding array element to sync" << i;
			if(!srcArray[i].isUndefined() &&
			   !srcArray[i].isObject())
				fatal("Invalid element in src array - Only synchronization of json arrays made up of json objects are supported");
			QJsonObject srcObj = srcArray[i].toObject();
			QJsonObject resObj;
			performObjSync(srcObj, {}, resObj);
			srcArray[i] = srcObj;
			resArray.append(resObj);
			_syncNeedsUpdate = true;
		}
	} else {//case: sync has more elements than src and is needed
		for(; i < arrayMax; ++i) {
			info() << "Adding array element to src" << i;
			if(!syncArray[i].isUndefined() &&
			   !syncArray[i].isObject())
				fatal("Invalid element in sync array - Only synchronization of json arrays made up of json objects are supported");
			QJsonObject srcObj;
			QJsonObject resObj;
			performObjSync(srcObj, syncArray[i].toObject(), resObj);
			srcArray.append(srcObj);
			resArray.append(resObj);
			_srcNeedsUpdate = true;
		}
	}
}

void JsonSyncTask::performObjSync(QJsonObject &srcObj, const QJsonObject &syncObj, QJsonObject &resObj)
{
	for(const auto &keyChain : qAsConst(_keyChains))
		traverse(srcObj, syncObj, resObj, keyChain, 0);
}

void JsonSyncTask::traverse(QJsonObject &srcObj, const QJsonObject &syncObj, QJsonObject &resObj, const QStringList &keyChain, int keyIndex)
{
	// case last chain -> sync the entry
	Q_ASSERT(!keyChain.isEmpty());
	if(keyIndex == keyChain.size() - 1) {
		syncElement(srcObj, syncObj, resObj,
					keyChain.last(),
					keyChain.join(QLatin1Char('/')));
		return;
	}

	// normal traversal
	const auto &key = keyChain[keyIndex];
	auto srcChild = getChild(srcObj, key, keyChain, keyIndex, "src");
	auto syncChild = getChild(syncObj, key, keyChain, keyIndex, "sync");
	auto resChild = getChild(resObj, key, keyChain, keyIndex, "sync");
	// traverse to next layer and copy back resulting objects into the (current) parent
	traverse(srcChild, syncChild, resChild, keyChain, keyIndex + 1);
	srcObj.insert(key, srcChild);
	resObj.insert(key, resChild);
}

QJsonObject JsonSyncTask::getChild(const QJsonObject &srcObj, const QString &key, const QStringList &keyChain, int keyIndex, const QByteArray &target)
{
	const auto childValue = srcObj.value(key);
	if(!childValue.isUndefined() &&
	   !childValue.isObject()) {
		fatal("Invalid intermediate element in " + target + " that is not a json object",
			  keyChain.mid(0, keyIndex + 1).join(QLatin1Char('/')));
	}
	return childValue.toObject();
}

void JsonSyncTask::syncElement(QJsonObject &srcObj, const QJsonObject &syncObj, QJsonObject &resObj, const QString &key, const QString &logKey)
{
	if(srcObj.contains(key)) {
		if(syncObj.contains(key)) {
			if(srcObj[key] == syncObj[key]) {
				resObj.insert(key, syncObj[key]);
				debug(logKey) << "Skipping unchanged entry";
			} else if(_srcIsNewer) {
				resObj.insert(key, srcObj[key]);
				_syncNeedsUpdate = true;
				info(logKey) << "Updated entry in sync from src";
			} else {
				srcObj.insert(key, syncObj[key]);
				resObj.insert(key, syncObj[key]);
				_srcNeedsUpdate = true;
				info(logKey) << "Updated entry in src from sync";
			}
		} else {
			resObj.insert(key, srcObj[key]);
			_syncNeedsUpdate = true;
			info(logKey) << "Added entry to sync from src";
		}
	} else if(syncObj.contains(key)) {
		srcObj.insert(key, syncObj[key]);
		resObj.insert(key, syncObj[key]);
		_srcNeedsUpdate = true;
		info(logKey) << "Added entry to src from sync";
	} // else: neither have the entry, so do nothing
}

QJsonDocument JsonSyncTask::readFile(const QString &path, const QByteArray &target)
{
	QFile file{path};
	if(!file.exists())
		return {};

	if(!file.open(QIODevice::ReadOnly | (_isBinary ? QIODevice::NotOpen : QIODevice::Text))) {//NotOpen is the 0 flag
		fatal("Failed to open " + target + " file for reading with error: " +
			  file.errorString().toUtf8());
	}

	QJsonDocument doc;
	if(_isBinary) {
		doc = QJsonDocument::fromBinaryData(file.readAll(), QJsonDocument::Validate);
		if(doc.isNull())
			fatal("Failed to read binary json from " + target + " - data is not valid");
	} else {
		QJsonParseError error;
		doc = QJsonDocument::fromJson(file.readAll(), &error);
		if(error.error != QJsonParseError::NoError)
			fatal("Failed to read json from " + target + " with error:" +
				  error.errorString().toUtf8());
	}

	file.close();
	return doc;
}

void JsonSyncTask::writeFile(const QString &path, const QJsonDocument &doc, const QByteArray &target)
{
	QSaveFile file{path};
	if(!file.open(QIODevice::WriteOnly | (_isBinary ? QIODevice::NotOpen : QIODevice::Text))) {//NotOpen is the 0 flag
		fatal("Failed to open " + target + " file for writing with error: " +
			  file.errorString().toUtf8());
	}

	if(_isBinary)
		file.write(doc.toBinaryData());
	else
		file.write(doc.toJson(QJsonDocument::Indented));

	if(!file.commit()) {//NotOpen is the 0 flag
		fatal("Failed write to " + target + " file with error: " +
			  file.errorString().toUtf8());
	}
}
