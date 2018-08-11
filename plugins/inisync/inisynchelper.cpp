#include "inisynchelper.h"
#include <QDebug>
#include <QDateTime>
#include <QSaveFile>
#include <algorithm>

const QString IniSyncHelper::ModeIni = QStringLiteral("ini");

IniSyncHelper::IniSyncHelper(QObject *parent) :
	SyncHelper(parent)
{}

QString IniSyncHelper::syncPrefix() const
{
	return QStringLiteral("ini");
}

bool IniSyncHelper::pathIsPattern(const QString &mode) const
{
	Q_UNUSED(mode)
	return true;
}

bool IniSyncHelper::canSyncDirs(const QString &mode) const
{
	Q_UNUSED(mode)
	return false;
}

void IniSyncHelper::performSync(const QString &path, const QString &mode, const QStringList &extras, bool isFirstUse)
{
	if(mode != ModeIni)
		throw SyncException("Unsupported path mode");

	QFileInfo srcInfo, syncInfo;
	std::tie(srcInfo, syncInfo) = generatePaths(path);

	KeyInfo subKeys;
	subKeys.first.reserve(extras.size());
	subKeys.second.reserve(extras.size());
	for(const auto& extra : extras) {
		if(extra.startsWith(QLatin1Char('!')))
			subKeys.second.append(extra.midRef(1).toUtf8().split('\\'));
		else
			subKeys.first.append(extra.toUtf8().split('\\'));
	}

	// step 1: read and map the current sync state
	auto srcIsNewer = isFirstUse ? false : srcInfo.lastModified() > syncInfo.lastModified();
	auto updateMapping = createMapping(syncInfo);
	auto workMapping = updateMapping;

	// step 2: prepare the files
	QFile srcRead(srcInfo.absoluteFilePath()); //read first, as write may create it if not existing
	if(srcInfo.exists()) {
		if(!srcRead.open(QIODevice::ReadOnly | QIODevice::Text)) {
			throw SyncException("Failed to open sync file for reading with error: " +
								srcRead.errorString().toUtf8());
		}
	}
	QSaveFile srcWrite(srcInfo.absoluteFilePath());
	srcWrite.setDirectWriteFallback(false);
	if(!srcWrite.open(QIODevice::WriteOnly | QIODevice::Text)) {
		throw SyncException("Failed to open src file for save writing with error: " +
							srcWrite.errorString().toUtf8());
	}

	// step 3: update the src and sync from each other
	auto srcNeedsSave = false;
	auto syncNeedsSave = false;
	auto writeFirstLine = true;
	auto holdingNewLine = false;
	auto writeGroupRemainder = [&](const QByteArray &cGroup) -> bool{
		auto written = false;
		for(auto it = workMapping[cGroup].constBegin(); it != workMapping[cGroup].constEnd(); it++) {
			//only handle lines that should be synced
			if(shouldSync(cGroup, it.key(), subKeys)) {
				srcWrite.write(it.value());
				srcNeedsSave = true;
				written = true;
				log(srcInfo, "Added new entry from sync to src", cGroup, it.key());
			} else if(updateMapping[cGroup].remove(it.key()) > 0) {
				syncNeedsSave = true;
				log(srcInfo, "Removed non-syncable entrie from sync", cGroup, it.key());
			}
		}
		workMapping.remove(cGroup);
		return written;
	};
	if(srcInfo.exists()) {
		QByteArray cGroup;
		while(!srcRead.atEnd()) {
			auto line = srcRead.readLine();
			auto sLine = line.simplified();
			// handle newlines -> don't write them until needed
			if(sLine.isEmpty() && !holdingNewLine) {
				holdingNewLine = true;
				continue;
			// handle group lines
			} else if(sLine.startsWith('[') && sLine.endsWith(']')) {
				// copy all unsynced entries left in the group
				auto written = writeGroupRemainder(cGroup);
				if(written) {
					srcWrite.write("\n");
					holdingNewLine = false;
				}
				// begin the new group
				cGroup = sLine.mid(1).chopped(1);
			// handle normal lines
			} else {
				//special case: write hold back newlines
				if(holdingNewLine) {
					srcWrite.write("\n");
					holdingNewLine = false;
				}

				auto index = line.indexOf('=');
				if(index >= 0) { // line is syncable
					auto key = line.mid(0, index);
					//check if should sync
					if(shouldSync(cGroup, key, subKeys)) {
						// check if already synced
						auto workValue = workMapping[cGroup].value(key);
						if(!workValue.isNull()) {
							// only sync if actually different
							if(workValue != line) {
								// update sync from source
								if(srcIsNewer) {
									updateMapping[cGroup].insert(key, line);
									syncNeedsSave = true;
									log(srcInfo, "Updated entry in sync from src", cGroup, key);
								// update src from sync
								} else {
									line = workValue;
									srcNeedsSave = true;
									log(srcInfo, "Updated entry in src from sync", cGroup, key);
								}
							} else
								log(srcInfo, "Skipping unchanged entry", cGroup, key, true);
							// remove anyways, has been handeled
							workMapping[cGroup].remove(key);
						// if not existing, sync it
						} else {
							updateMapping[cGroup].insert(key, line);
							syncNeedsSave = true;
							log(srcInfo, "Added new entry from src to sync", cGroup, key);
						}
					// else: do nothing (aka just copy the line) but remove from sync if existing
					} else if(updateMapping[cGroup].remove(key) > 0) {
						workMapping[cGroup].remove(key); //has been handeled
						syncNeedsSave = true;
						log(srcInfo, "Removed non-syncable entrie from sync", cGroup, key);
					}
				}
			}

			srcWrite.write(line);
			writeFirstLine = false;
		}

		// copy all unsynced entries left in the final group
		writeGroupRemainder(cGroup);

		srcRead.close();
	}

	// step 4: write all unhandeled synced groups and then save the src file if neccessary
	writeMapping(&srcWrite, workMapping, writeFirstLine, srcNeedsSave, srcInfo.absoluteFilePath());
	if(srcNeedsSave) {
		if(!srcWrite.commit())
			throw SyncException("Failed to save src file with error: " +
								srcWrite.errorString().toUtf8());
	} else {
		srcWrite.cancelWriting();
		log(srcInfo, "No new synced changes, not update src", true);
	}

	// step 5: write the sync if needed
	if(syncNeedsSave)
		writeMapping(syncInfo, updateMapping);
	else
		log(srcInfo, "No new src changes, not update sync", true);
}

void IniSyncHelper::undoSync(const QString &path, const QString &mode)
{
	if(mode != ModeIni)
		throw SyncException("Unsupported path mode");
	removeSyncPath(path, "INI-SYNC");
}

SyncHelper::ExtrasHint IniSyncHelper::extrasHint() const
{
	return {
		true,
		tr("Keys"),
		tr("<p>Enter the keys you want to synchronize. All entries that start with the given keys will be synchronized."
		   "You can also enter \"inverted\" keys to exclude entries.</p>"
		   "<p>For example, adding the keys"
		   "<ul>"
		   "	<li>test\\group</li>"
		   "	<li>!test\\group\\child</li>"
		   "	<li>test\\group\\child\\special</li>"
		   "</ul>"
		   "Will synchronize every entry that begins with \"test\\group\", but exclude all entries from \"test\\group\\child\", "
		   "except everything that starts with \"test\\group\\child\\special\".</p>")
	};
}

IniSyncHelper::IniEntryMapping IniSyncHelper::createMapping(const QFileInfo &file) const
{
	if(!file.exists())
		return {};

	QFile readFile(file.absoluteFilePath());
	if(!readFile.open(QIODevice::ReadOnly)) { //open in binary mode - the synced file has linux fileendings
		throw SyncException("Failed to open sync file for reading with error: " +
							readFile.errorString().toUtf8());
	}

	IniEntryMapping mapping;
	QByteArray cGroup;
	while(!readFile.atEnd()) {
		auto line = readFile.readLine();
		auto sLine = line.simplified();
		if(sLine.startsWith('[') && sLine.endsWith(']'))
			cGroup = sLine.mid(1).chopped(1);
		else {
			auto index = line.indexOf('=');
			if(index >= 0)
				mapping[cGroup].insert(line.mid(0, index), line);
		}
	}

	readFile.close();
	return mapping;
}

void IniSyncHelper::writeMapping(QIODevice *device, const IniSyncHelper::IniEntryMapping &mapping, bool firstLine, bool &needSave, const QString &logStr) const
{
	// emtpy group will always be first in the map due to the sorting
	for(auto it = mapping.constBegin(); it != mapping.constEnd(); it++) {
		// write the group
		if(firstLine) {
			if(!it.key().isEmpty())
				device->write("[" + it.key() + "]\n");
			//else: empty cgroup means no grp at all, so skip it
		} else
			device->write("\n[" + it.key() + "]\n"); //an emtpy cgroup not in the first line will produce "[]" - not ideal, but ok, as it does not change the behaviour
		firstLine = false;

		// write all entries
		const auto& subMap = it.value();
		for(auto jt = subMap.constBegin(); jt != subMap.constEnd(); jt++) {
			device->write(jt.value());
			needSave = true;
			if(!logStr.isNull())
				log(logStr, "Updated entry in src from sync", it.key(), jt.key());
		}
	}
}

void IniSyncHelper::writeMapping(const QFileInfo &file, const IniSyncHelper::IniEntryMapping &mapping)
{
	QSaveFile writeFile(file.absoluteFilePath());
	if(!writeFile.open(QIODevice::WriteOnly)) { //open in binary mode - the synced file has linux fileendings
		throw SyncException("Failed to open sync file for writing with error: " +
							writeFile.errorString().toUtf8());
	}

	auto needSave = false;
	writeMapping(&writeFile, mapping, true, needSave);
	if(!writeFile.commit()) {
		throw SyncException("Failed to save sync file with error: " +
							writeFile.errorString().toUtf8());
	}
}

bool IniSyncHelper::shouldSync(const QByteArray &group, const QByteArray &key, const KeyInfo &extras) const
{
	QByteArrayList keyList = key.split('\\');
	if(!group.isEmpty())
		keyList.prepend(group);
	for(const auto& extra : extras.first) {
		if(startsWith(keyList, extra)) {
			auto exclude = false;
			for(const auto &noExtra : extras.second) {
				if(noExtra.size() > extra.size() &&
				   startsWith(keyList, noExtra)) {
					exclude = true;
					break;
				}
			}
			if(!exclude)
				return true;
		}
	}
	return false;
}

bool IniSyncHelper::startsWith(const QByteArrayList &key, const QByteArrayList &subList) const
{
	if(subList.size() > key.size())
		return false;
	return std::equal(subList.begin(), subList.end(), key.begin());
}

void IniSyncHelper::log(const QFileInfo &file, const char *msg, bool dbg) const
{
	(dbg ? qDebug() : qInfo()).noquote() << "INI-SYNC:" << file.absoluteFilePath() << "=>" << msg;
}

void IniSyncHelper::log(const QFileInfo &file, const char *msg, const QByteArray &cGroup, const QByteArray &key, bool dbg) const
{
	(dbg ? qDebug() : qInfo()).noquote() << "INI-SYNC:" << file.absoluteFilePath()
										 << "=>" << msg << ('[' + (cGroup.isEmpty() ? key : QByteArray{cGroup + '\\' + key}) + ']');
}
