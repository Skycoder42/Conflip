#include "pathsynchelper.h"
#include <QRegularExpression>
#include <QDebug>
#include <QDateTime>
#include <QCryptographicHash>
#include <QStorageInfo>

const QString PathSyncHelper::ModeSymlink = QStringLiteral("symlink");
const QString PathSyncHelper::ModeCopy = QStringLiteral("copy");

PathSyncHelper::PathSyncHelper(QObject *parent) :
	SyncHelper(parent),
	_regexCache(100000)
{}

QString PathSyncHelper::syncPrefix() const
{
	return QStringLiteral("files");
}

bool PathSyncHelper::pathIsPattern(const QString &mode) const
{
	Q_UNUSED(mode)
	return true;
}

bool PathSyncHelper::canSyncDirs(const QString &mode) const
{
	return mode == QStringLiteral("symlink"); //only symlink can sync dirs...
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
	std::tie(srcInfo, syncInfo) = generatePaths(path);

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
		std::tie(srcInfo, syncInfo) = generatePaths(path);
		unlink(srcInfo, syncInfo);
	} else if(mode == ModeCopy)
		removeSyncPath(path, "PATH-SYNC");
	else
		throw SyncException("Unsupported path mode");
}

void PathSyncHelper::syncAsSymlink(const QFileInfo &src, const QFileInfo &sync, bool isFirstUse)
{
	// if syncfile does not exist -> move src to sync
	if(!sync.exists()) {
		if(!src.exists())
			return;
		movePath(src, sync, true);
		log(src, "Moved file from src to sync folder");
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
		Q_ASSERT(!src.isSymLink()); //should be impossible
		if(isFirstUse) {
			if(!removePath(src))
				throw SyncException("Failed to remove src for first sync");
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
	// dirs are currently not possible as a copy
	if(src.isDir() || sync.isDir())
		throw SyncException("Cannot copy-sync directories! Use symlink sync or wildcard paths");

	// if syncfile does not exist - create it
	if(!sync.exists()) {
		if(!src.exists())
			return;

		if(!QFile::copy(src.absoluteFilePath(), sync.absoluteFilePath()))
			throw SyncException("Failed to copy src to sync");
		log(src, "Copied src to sync folder");
		return;
	}

	// check if exists
	if(src.exists()) {
		if(isFirstUse) {
			if(!QFile::remove(src.absoluteFilePath()))
				throw SyncException("Failed to remove src file for first sync");
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
				log(src, "Copied src to sync folder");
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
	log(src, "Copied sync to src");
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

	movePath(sync, src, false);
	log(src, "Removed file from synchronisation");
}

void PathSyncHelper::movePath(const QFileInfo &from, const QFileInfo &to, bool fromIsSrc)
{
	Q_ASSERT(!to.exists());
	const auto fromName = fromIsSrc ?
							  QByteArrayLiteral("src") :
							  QByteArrayLiteral("sync");
	const auto toName = fromIsSrc ?
							  QByteArrayLiteral("sync") :
							  QByteArrayLiteral("src");
	const QByteArray direction = fromName + " to " + toName;

	if(from.isSymLink()) {
		throw SyncException("Failed to move " + direction + ", because " + fromName +
							" is a symlink (and symlinks cannot be symlink-synced)");
	}

	if(from.isDir()) { //dir/symlink can only be truly moved, not copy-moved
		QStorageInfo fromInfo{from.absoluteFilePath()};
		QStorageInfo toInfo{to.dir().absolutePath()};

		if(!fromInfo.isValid())
			throw SyncException("Unable to detect volume of " + fromName);
		if(!toInfo.isValid())
			throw SyncException("Unable to detect volume of " + toName);
		if(fromInfo != toInfo)
			throw SyncException("Detected volumes of src and sync are not the same - cannot move directories between different volumes");

		QDir rootDir{fromInfo.rootPath()};
		if(!rootDir.rename(rootDir.relativeFilePath(from.absoluteFilePath()),
						   rootDir.relativeFilePath(to.absoluteFilePath()))) {
			throw SyncException("Failed to directory-move " + direction);
		}
	} else if(!QFile::rename(from.absoluteFilePath(), to.absoluteFilePath()))
		throw SyncException("Failed to move or copy-move " + direction);
}

bool PathSyncHelper::removePath(const QFileInfo &path)
{
	if(path.isDir())
		return QDir{path.absoluteFilePath()}.removeRecursively();
	else
		return QFile::remove(path.absoluteFilePath());
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
