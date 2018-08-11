#include "jsonsynchelper.h"
#include <QDateTime>
#include <QSaveFile>
#include <QDebug>

const QString JsonSyncHelper::ModeJson = QStringLiteral("json");
const QString JsonSyncHelper::ModeQbjs = QStringLiteral("qbjs");

JsonSyncHelper::JsonSyncHelper(bool isBinary, QObject *parent) :
	SyncHelper{parent},
	_isBinary{isBinary}
{}

QString JsonSyncHelper::syncPrefix() const
{
	return QStringLiteral("json");
}

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

void JsonSyncHelper::performSync(const QString &path, const QString &mode, const QStringList &extras, bool isFirstUse)
{
	if((_isBinary && mode != ModeQbjs) ||
	   (!_isBinary && mode != ModeJson))
		throw SyncException("Unsupported path mode");

	QFileInfo srcInfo, syncInfo;
	std::tie(srcInfo, syncInfo) = generatePaths(path);

	// step 0: prepare extras
	QList<QStringList> keyChains;
	keyChains.reserve(extras.size());
	for(const auto &extra : extras) {
		keyChains.append(extra.split(QLatin1Char('/'), QString::SkipEmptyParts));
		if(keyChains.last().isEmpty())
			throw SyncException("Empty key found in extras - thats not allowed");
	}

	// step 1: read both files, determine the mode and prepare it
	const auto srcIsNewer = isFirstUse ? false : srcInfo.lastModified() > syncInfo.lastModified();
	auto srcDoc = readFile(srcInfo.absoluteFilePath(), "src");
	const auto syncDoc = readFile(syncInfo.absoluteFilePath(), "sync");
	if((srcDoc.isObject() && syncDoc.isArray()) ||
	   (syncDoc.isObject() && srcDoc.isArray()))
		throw SyncException("Cannot synchronize files where one is a json array and the other a json object");

	// step 2: call sync methods, as array or as obj
	QJsonDocument resDoc;
	auto srcNeedsUpdate = false;
	auto syncNeedsUpdate = false;
	if(srcDoc.isArray() || syncDoc.isArray()) {
		auto arrayIsNewer = srcIsNewer;
		if(!srcDoc.isArray())
			arrayIsNewer = false;
		if(!syncDoc.isArray())
			arrayIsNewer = true;
		auto srcArray = srcDoc.array();
		const auto syncArray = syncDoc.array();
		QJsonArray resArray;
		performArraySync(srcArray, syncArray, resArray, keyChains, arrayIsNewer, srcNeedsUpdate, syncNeedsUpdate, path);
		srcDoc.setArray(srcArray);
		resDoc.setArray(resArray);
	} else {
		auto srcObj = srcDoc.object();
		const auto syncObj = syncDoc.object();
		QJsonObject resObj;
		performObjSync(srcObj, syncObj, resObj, keyChains, srcIsNewer, srcNeedsUpdate, syncNeedsUpdate, path);
		srcDoc.setObject(srcObj);
		resDoc.setObject(resObj);
	}

	// step 3: write the files that need changes
	if(srcNeedsUpdate)
		writeFile(srcInfo.absoluteFilePath(), srcDoc, "src");
	if(syncNeedsUpdate)
		writeFile(syncInfo.absoluteFilePath(), resDoc, "sync");
}

void JsonSyncHelper::undoSync(const QString &path, const QString &mode)
{
	if((_isBinary && mode != ModeQbjs) ||
	   (!_isBinary && mode != ModeJson))
		throw SyncException("Unsupported path mode");

	removeSyncPath(path, "JSON-SYNC");
}

SyncHelper::ExtrasHint JsonSyncHelper::extrasHint() const
{
	return {
		true,
		tr("Keys"),
		tr("<p>Enter the key-path you want to synchronize. The path is a chain of keys of properties that "
		   "are traversed into the object and the last in the chain is synchronized. The keys are seperated "
		   "by a '/'.</p>"
		   "<p>For example, the key \"group/key\" would enter the property named \"group\" in the root element "
		   "and there get the value of the \"key\" property and synchronize it.</p>")
	};
}

void JsonSyncHelper::performArraySync(QJsonArray &srcArray, const QJsonArray &syncArray, QJsonArray &resArray, const QList<QStringList> &keyChains, bool srcIsNewer, bool &srcNeedsUpdate, bool &syncNeedsUpdate, const QString &logPath)
{
	Q_ASSERT(resArray.isEmpty());
	// step 1: trim src array to max size to handle cases where src is longer but not wanted, and also log that
	auto arrayMax = srcIsNewer ? srcArray.size() : syncArray.size();
	if(srcArray.size() > arrayMax) {
		log(logPath, QByteArray{"Removed " + QByteArray::number(srcArray.size() - arrayMax) + " elements from src array"}.constData());
		while(srcArray.size() > arrayMax)
			srcArray.removeLast();
		srcNeedsUpdate = true;
	}
	if(syncArray.size() > arrayMax) {
		log(logPath, QByteArray{"Removed " + QByteArray::number(syncArray.size() - arrayMax) + " elements from sync array"}.constData());
		syncNeedsUpdate = true;
	}

	// step 2: go through all elements that are in both arrays and sync them
	auto i = 0;
	for(auto nMax = std::min(srcArray.size(), syncArray.size()); i < nMax; ++i) {
		log(logPath, "Operating on array element", QByteArray::number(i));
		if(!srcArray[i].isUndefined() &&
		   !srcArray[i].isObject())
			throw SyncException("Invalid element in src array - Only synchronization of json arrays made up of json objects are supported");
		if(!syncArray[i].isUndefined() &&
		   !syncArray[i].isObject())
			throw SyncException("Invalid element in sync array - Only synchronization of json arrays made up of json objects are supported");
		QJsonObject srcObj = srcArray[i].toObject();
		QJsonObject resObj;
		performObjSync(srcObj, syncArray[i].toObject(), resObj, keyChains, srcIsNewer, srcNeedsUpdate, syncNeedsUpdate, logPath);
		srcArray[i] = srcObj;
		resArray.append(resObj);
	}

	// step 3: sync the remaining elements that only exist in 1 list (only happens if wanted is the longer array)
	if(srcIsNewer) { //case: src has more elements than sync and is needed
		for(; i < arrayMax; ++i) {
			log(logPath, "Adding array element to sync", QByteArray::number(i));
			if(!srcArray[i].isUndefined() &&
			   !srcArray[i].isObject())
				throw SyncException("Invalid element in src array - Only synchronization of json arrays made up of json objects are supported");
			QJsonObject srcObj = srcArray[i].toObject();
			QJsonObject resObj;
			performObjSync(srcObj, {}, resObj, keyChains, srcIsNewer, srcNeedsUpdate, syncNeedsUpdate, logPath);
			srcArray[i] = srcObj;
			resArray.append(resObj);
			syncNeedsUpdate = true;
		}
	} else {//case: sync has more elements than src and is needed
		for(; i < arrayMax; ++i) {
			log(logPath, "Adding array element to src", QByteArray::number(i));
			if(!syncArray[i].isUndefined() &&
			   !syncArray[i].isObject())
				throw SyncException("Invalid element in sync array - Only synchronization of json arrays made up of json objects are supported");
			QJsonObject srcObj;
			QJsonObject resObj;
			performObjSync(srcObj, syncArray[i].toObject(), resObj, keyChains, srcIsNewer, srcNeedsUpdate, syncNeedsUpdate, logPath);
			srcArray.append(srcObj);
			resArray.append(resObj);
			srcNeedsUpdate = true;
		}
	}
}

void JsonSyncHelper::performObjSync(QJsonObject &srcObj, const QJsonObject &syncObj, QJsonObject &resObj, const QList<QStringList> &keyChains, bool srcIsNewer, bool &srcNeedsUpdate, bool &syncNeedsUpdate, const QString &logPath)
{
	for(const auto &keyChain : keyChains)
		traverse(srcObj, syncObj, resObj, keyChain, 0, srcIsNewer, srcNeedsUpdate, syncNeedsUpdate, logPath);
}

void JsonSyncHelper::traverse(QJsonObject &srcObj, const QJsonObject &syncObj, QJsonObject &resObj, const QStringList &keyChain, int keyIndex, bool srcIsNewer, bool &srcNeedsUpdate, bool &syncNeedsUpdate, const QString &logPath)
{
	// case last chain -> sync the entry
	Q_ASSERT(!keyChain.isEmpty());
	if(keyIndex == keyChain.size() - 1) {
		syncElement(srcObj, syncObj, resObj,
					keyChain.last(),
					srcIsNewer, srcNeedsUpdate, syncNeedsUpdate,
					logPath, keyChain.join(QLatin1Char('/')).toUtf8());
		return;
	}

	// normal traversal
	const auto &key = keyChain[keyIndex];
	auto srcChild = getChild(srcObj, key, keyChain, keyIndex, "src");
	auto syncChild = getChild(syncObj, key, keyChain, keyIndex, "sync");
	auto resChild = getChild(resObj, key, keyChain, keyIndex, "sync");
	// traverse to next layer and copy back resulting objects into the (current) parent
	traverse(srcChild, syncChild, resChild, keyChain, keyIndex + 1, srcIsNewer, srcNeedsUpdate, syncNeedsUpdate, logPath);
	srcObj.insert(key, srcChild);
	resObj.insert(key, resChild);
}

QJsonObject JsonSyncHelper::getChild(const QJsonObject &obj, const QString &key, const QStringList &keyChain, int keyIndex, const QByteArray &target)
{
	const auto childValue = obj.value(key);
	if(!childValue.isUndefined() &&
	   !childValue.isObject())
		throw SyncException{"Invalid intermediate element in " + target + " that is not a json object [" +
							keyChain.mid(0, keyIndex + 1).join(QLatin1Char('/')).toUtf8() + ']'};
	return childValue.toObject();
}

void JsonSyncHelper::syncElement(QJsonObject &srcObj, const QJsonObject &syncObj, QJsonObject &resObj, const QString &key, bool srcIsNewer, bool &srcNeedsUpdate, bool &syncNeedsUpdate, const QString &logPath, const QByteArray &logKey)
{
	if(srcObj.contains(key)) {
		if(syncObj.contains(key)) {
			if(srcObj[key] == syncObj[key]) {
				resObj.insert(key, syncObj[key]);
				log(logPath, "Skipping unchanged entry", logKey, true);
			} else if(srcIsNewer) {
				resObj.insert(key, srcObj[key]);
				syncNeedsUpdate = true;
				log(logPath, "Updated entry in sync from src", logKey);
			} else {
				srcObj.insert(key, syncObj[key]);
				resObj.insert(key, syncObj[key]);
				srcNeedsUpdate = true;
				log(logPath, "Updated entry in src from sync", logKey);
			}
		} else {
			resObj.insert(key, srcObj[key]);
			syncNeedsUpdate = true;
			log(logPath, "Added entry to sync from src", logKey);
		}
	} else if(syncObj.contains(key)) {
		srcObj.insert(key, syncObj[key]);
		resObj.insert(key, syncObj[key]);
		srcNeedsUpdate = true;
		log(logPath, "Added entry to src from sync", logKey);
	} // else: neither have the entry, so do nothing
}

QJsonDocument JsonSyncHelper::readFile(const QString &path, const QByteArray &target)
{
	QFile file{path};
	if(!file.exists())
		return {};

	if(!file.open(QIODevice::ReadOnly | (_isBinary ? QIODevice::NotOpen : QIODevice::Text))) {//NotOpen is the 0 flag
		throw SyncException{"Failed to open " + target + " file for reading with error: " +
							file.errorString().toUtf8()};
	}

	QJsonDocument doc;
	if(_isBinary) {
		doc = QJsonDocument::fromBinaryData(file.readAll(), QJsonDocument::Validate);
		if(doc.isNull())
			throw SyncException{"Failed to read binary json from " + target + " - data is not valid"};
	} else {
		QJsonParseError error;
		doc = QJsonDocument::fromJson(file.readAll(), &error);
		if(error.error != QJsonParseError::NoError)
			throw SyncException{"Failed to read json from " + target + " with error:" +
								error.errorString().toUtf8()};
	}

	file.close();
	return doc;
}

void JsonSyncHelper::writeFile(const QString &path, const QJsonDocument &doc, const QByteArray &target)
{
	QSaveFile file{path};
	if(!file.open(QIODevice::WriteOnly | (_isBinary ? QIODevice::NotOpen : QIODevice::Text))) {//NotOpen is the 0 flag
		throw SyncException{"Failed to open " + target + " file for writing with error: " +
							file.errorString().toUtf8()};
	}

	if(_isBinary)
		file.write(doc.toBinaryData());
	else
		file.write(doc.toJson(QJsonDocument::Indented));

	if(!file.commit()) {//NotOpen is the 0 flag
		throw SyncException{"Failed write to " + target + " file with error: " +
							file.errorString().toUtf8()};
	}
}

void JsonSyncHelper::log(const QString &path, const char *msg, bool dbg) const
{
	(dbg ? qDebug() : qInfo()).noquote() << "JSON-SYNC:" << path << "=>" << msg;
}

void JsonSyncHelper::log(const QString &path, const char *msg, const QByteArray &key, bool dbg) const
{
	(dbg ? qDebug() : qInfo()).noquote() << "JSON-SYNC:" << path << "=>" << msg << ('[' + key + ']');
}
