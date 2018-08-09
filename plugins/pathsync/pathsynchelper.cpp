#include "pathsynchelper.h"
#include <QRegularExpression>
#include <QDebug>
#include <QDateTime>
#include <QCryptographicHash>

const QString PathSyncHelper::ModeSymlink = QStringLiteral("symlink");
const QString PathSyncHelper::ModeCopy = QStringLiteral("copy");

PathSyncHelper::PathSyncHelper(QObject *parent) :
	SyncHelper(parent),
	_regexCache(100000)
{}

bool PathSyncHelper::pathIsPattern(const QString &mode) const
{
	Q_UNUSED(mode)
	return true;
}

void PathSyncHelper::performSync(const QString &path, const QString &mode, const QStringList &extras, bool isFirstUse)
{
	for(const auto& extra : extras) {
		auto regex = _regexCache.object(extra);
		auto needInsert = false;
		if(!regex) {
			regex = new QRegularExpression(extra,
										   QRegularExpression::DontCaptureOption | QRegularExpression::UseUnicodePropertiesOption);
			regex->optimize();
			needInsert = true;
		}

		auto match = regex->match(path);
		if(needInsert)
			_regexCache.insert(extra, regex);

		if(match.hasMatch()) {
			undoSync(path, mode); //if it was ever synced, stop syncing it now
			return;
		}
	}

	QFileInfo srcInfo, syncInfo;
	std::tie(srcInfo, syncInfo) = generatePaths(QStringLiteral("files"), path);

	if(mode == ModeSymlink)
		syncAsSymlink(srcInfo, syncInfo, isFirstUse);
	else if(mode == ModeCopy)
		syncAsCopy(srcInfo, syncInfo, isFirstUse);
	else
		throw SyncException("Unsupported path mode");
}

void PathSyncHelper::undoSync(const QString &path, const QString &mode)
{
	if(mode == ModeSymlink) {
		QFileInfo srcInfo, syncInfo;
		std::tie(srcInfo, syncInfo) = generatePaths(QStringLiteral("files"), path);
		unlink(srcInfo, syncInfo);
	} else if(mode == ModeCopy)
		removeSyncPath(QStringLiteral("files"), path, "PATH-SYNC");
	else
		throw SyncException("Unsupported path mode");
}

void PathSyncHelper::syncAsSymlink(const QFileInfo &src, const QFileInfo &sync, bool isFirstUse)
{
	// if syncfile does not exist - create it
	if(!sync.exists()) {
		if(!src.exists())
			return;

		if(!QFile::rename(src.absoluteFilePath(), sync.absoluteFilePath())) {
			// move fails for different devices -> copy and delete instead
			if(!QFile::copy(src.absoluteFilePath(), sync.absoluteFilePath()))
				throw SyncException("Failed to move or copy-move src to sync");
			if(!QFile::remove(src.absoluteFilePath()))
				throw SyncException("Failed to remove original file after copying");
		}
		// no return, proceed as usual
		log(src, "Moved file from original logication to sync folder");
	}

	// check if symlink
	if(src.exists() && src.isSymLink()) {
		//verify it points to dst
		if(src.symLinkTarget() == sync.absoluteFilePath()) {
			log(src, "Symlink is intact", true);
			return; //all ok
		} else {
			//assume it's because the sync folder change it's location and replace it
			if(!QFile::remove(src.absoluteFilePath()))
				throw SyncException("Failed to remove invalid symlink");
			log(src, "Removed invalid symlink");
		}
	}

	// check if exists
	if(src.exists()) {
		if(isFirstUse) {
			if(!QFile::remove(src.absoluteFilePath()))
				throw SyncException("Failed to remove original file for first sync");
		} else {
			qWarning().noquote() << "PATH-SYNC:" << src.absoluteFilePath()
								 << "=> Symlink sync conflict - switching over to copy mode";
			syncAsCopy(src, sync, false); //assume no first use -> copy modified src
			throw NotASymlinkException();
		}
	}

	//does not exist -> create symlink
	if(!QFile::link(sync.absoluteFilePath(), src.absoluteFilePath()))
		throw SyncException("Failed to create symlink");
	log(src, "Created symlink");
}

void PathSyncHelper::syncAsCopy(const QFileInfo &src, const QFileInfo &sync, bool isFirstUse)
{
	// if syncfile does not exist - create it
	if(!sync.exists()) {
		if(!src.exists())
			return;

		if(!QFile::copy(src.absoluteFilePath(), sync.absoluteFilePath()))
			throw SyncException("Failed to copy source to sync");
		log(src, "Copied source to sync folder");
		return;
	}

	// check if exists
	if(src.exists()) {
		if(isFirstUse) {
			if(!QFile::remove(src.absoluteFilePath()))
				throw SyncException("Failed to remove original file for first sync");
		} else {
			if(hashFile(src) == hashFile(sync)) {
				log(src, "Files are the same", true);
				return; //files are the same
			}

			//else - compare timestamps
			if(src.lastModified() > sync.lastModified()) {
				// remove sync and replace with src
				if(!QFile::remove(sync.absoluteFilePath()))
					throw SyncException("Failed to remove older sync file");
				if(!QFile::copy(src.absoluteFilePath(), sync.absoluteFilePath()))
					throw SyncException("Failed to copy src to sync");
				log(src, "Copied source to sync folder");
				return;
			} else { //delete to enter not exist state
				if(!QFile::remove(src.absoluteFilePath()))
					throw SyncException("Failed to remove older src file");
			}
		}
	}

	//does not exist -> copy it
	if(!QFile::copy(sync.absoluteFilePath(), src.absoluteFilePath()))
		throw SyncException("Failed to copy sync to src");
	log(src, "Copied from sync folder to source");
}

void PathSyncHelper::unlink(const QFileInfo &src, const QFileInfo &sync)
{
	//only do if valid symlink
	if(!src.isSymLink())
		return;
	if(src.symLinkTarget() != sync.absoluteFilePath())
		return;

	if(!QFile::remove(src.absoluteFilePath()))
		throw SyncException("Failed to remove old symlink");

	if(!QFile::rename(sync.absoluteFilePath(), src.absoluteFilePath())) {
		// move fails for different devices -> copy and delete instead
		if(!QFile::copy(sync.absoluteFilePath(), src.absoluteFilePath()))
			throw SyncException("Failed to move or copy-move sync to src");
		if(!QFile::remove(sync.absoluteFilePath())) {
			qWarning().noquote() << "PATH-SYNC:" << src.absoluteFilePath()
								 << "=> Failed to remove sync file after restoring original file";
		}
	}

	log(src, "Removed file from synchronisation");
}

QByteArray PathSyncHelper::hashFile(const QFileInfo &file) const
{
	QCryptographicHash hash(QCryptographicHash::Sha3_512);
	QFile hashFile(file.absoluteFilePath());
	if(!hashFile.open(QIODevice::ReadOnly))
		throw SyncException("Unable to open file for reading to calculate hashsum");
	hash.addData(&hashFile);
	return hash.result();
}

void PathSyncHelper::log(const QFileInfo &file, const char *msg, bool dbg) const
{
	(dbg ? qDebug() : qInfo()).noquote() << "PATH-SYNC:" << file.absoluteFilePath() << "=>" << msg;
}
