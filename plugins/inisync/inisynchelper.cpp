#include "inisynchelper.h"
#include <QDebug>
#include <QDateTime>
#include <QSaveFile>

const QString IniSyncHelper::ModeIni = QStringLiteral("ini");

IniSyncHelper::IniSyncHelper(QObject *parent) :
	SyncHelper(parent)
{}

bool IniSyncHelper::pathIsPattern(const QString &mode) const
{
	Q_UNUSED(mode)
	return true;
}

void IniSyncHelper::performSync(const QString &path, const QString &mode, const QStringList &extras, bool isFirstUse)
{
	if(mode != ModeIni)
		throw SyncException("Unsupported path mode");

	QFileInfo srcInfo, syncInfo;
	std::tie(srcInfo, syncInfo) = generatePaths(QStringLiteral("ini"), path);

	QByteArrayList subKeys;
	for(auto extra : extras)
		subKeys.append(extra.toUtf8());

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
				auto written = false;
				for(auto it = workMapping[cGroup].constBegin(); it != workMapping[cGroup].constEnd(); it++) {
					srcWrite.write(it.value());
					srcNeedsSave = true;
					written = true;
					log(srcInfo, "Added new entry from sync to src", cGroup, it.key());
				}
				workMapping.remove(cGroup);
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
					// check if wanted to be synced
					} else if(shouldSync(cGroup, key, subKeys)) {
						updateMapping[cGroup].insert(key, line);
						syncNeedsSave = true;
						log(srcInfo, "Added new entry from src to sync", cGroup, key);
					}
					// else: do nothing (aka just copy the line)
				}
			}

			srcWrite.write(line);
			writeFirstLine = false;
		}

		// copy all unsynced entries left in the final group
		if(!cGroup.isNull()) {
			for(auto it = workMapping[cGroup].constBegin(); it != workMapping[cGroup].constEnd(); it++) {
				srcWrite.write(it.value());
				srcNeedsSave = true;
				log(srcInfo, "Added new entry from sync to src", cGroup, it.key());
			}
			workMapping.remove(cGroup);
		}

		srcRead.close();
	}

	// step 4: write all unhandeled synced groups and then save the src file if neccessary
	writeMapping(&srcWrite, workMapping, writeFirstLine, srcNeedsSave);
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

void IniSyncHelper::writeMapping(QIODevice *device, const IniSyncHelper::IniEntryMapping &mapping, bool firstLine, bool &needSave) const
{
	for(auto it = mapping.constBegin(); it != mapping.constEnd(); it++) {
		// write the group
		if(firstLine) {
			device->write("[" + it.key() + "]\n");
			firstLine = false;
		} else
			device->write("\n[" + it.key() + "]\n");

		// write all entries
		for(auto entry : it.value()) {
			device->write(entry);
			needSave = true;
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

bool IniSyncHelper::shouldSync(const QByteArray &group, const QByteArray &key, const QByteArrayList &extras) const
{
	QByteArray tKey = group + '\\' + key;
	for(auto extra : extras) {
		if(tKey.startsWith(extra))
			return true;
	}
	return false;
}

void IniSyncHelper::log(const QFileInfo &file, const char *msg, bool dbg) const
{
	(dbg ? qDebug() : qInfo()).noquote() << "INI-SYNC:" << file.absoluteFilePath() << "=>" << msg;
}

void IniSyncHelper::log(const QFileInfo &file, const char *msg, const QByteArray &cGroup, const QByteArray &key, bool dbg) const
{
	(dbg ? qDebug() : qInfo()).noquote() << "INI-SYNC:" << file.absoluteFilePath()
										 << "=>" << msg << ('[' + cGroup + '\\' + key + ']');
}
