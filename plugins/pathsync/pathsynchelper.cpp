#include "pathsynchelper.h"
#include <QRegularExpression>
#include <QDebug>
#include <QDateTime>
#include <QCryptographicHash>
#include <QStorageInfo>

const QString PathSyncHelper::ModeSymlink = QStringLiteral("symlink");
const QString PathSyncHelper::ModeCopy = QStringLiteral("copy");

PathSyncHelper::PathSyncHelper(QObject *parent) :
	SyncHelper{parent}
{}

QString PathSyncHelper::syncPrefix(const QString &mode) const
{
	Q_UNUSED(mode)
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

SyncHelper::ExtrasHint PathSyncHelper::extrasHint() const
{
	return {
		true,
		tr("Excluded Paths"),
		tr("<p>You can enter regular expressions that are matched against the found paths to exclude them from synchronization. "
		   "The regex must be a standard PCRE-expression and can contain unicode characters</p>"
		   "<p>For example, assuming your path pattern is \"dir/file_*.txt\" and your directory contains the files"
		   "<ul>"
		   "	<li>file_tree.txt</li>"
		   "	<li>file_house.txt</li>"
		   "	<li>file_heart.txt</li>"
		   "</ul>"
		   "And you add the exclude pattern \"_h\\w+e.*\\.\", it will only match \"file_tree.txt\" and exclude the other two</p>")
	};
}

SyncTask *PathSyncHelper::createSyncTask(QString mode, const QDir &syncDir, QString path, QStringList extras, bool isFirstUse, QObject *parent)
{
	return new PathSyncTask {
		this,
		std::move(mode),
		syncDir,
		std::move(path),
		std::move(extras),
		isFirstUse,
		parent
	};
}

SyncTask *PathSyncHelper::createUndoSyncTask(QString mode, const QDir &syncDir, QString path, QObject *parent)
{
	return new PathSyncTask {
		this,
		std::move(mode),
		syncDir,
		std::move(path),
		parent
	};
}



PathSyncTask::PathSyncTask(const PathSyncHelper *helper, QString &&mode, const QDir &syncDir, QString &&path, QStringList &&extras, bool isFirstUse, QObject *parent) :
	SyncTask{helper, std::move(mode), syncDir, std::move(path), std::move(extras), isFirstUse, parent},
	_regexCache{100000}
{}

PathSyncTask::PathSyncTask(const PathSyncHelper *helper, QString &&mode, const QDir &syncDir, QString &&path, QObject *parent) :
	SyncTask{helper, std::move(mode), syncDir, std::move(path), parent},
	_regexCache{100000}
{}

void PathSyncTask::performSync()
{
	for(const auto& extra : extras) {
		auto regex = _regexCache.object(extra);
		auto needInsert = false;
		if(!regex) {
			regex = new QRegularExpression {
				extra,
				QRegularExpression::DontCaptureOption | QRegularExpression::UseUnicodePropertiesOption
			};
			regex->optimize();
			needInsert = true;
		}

		auto match = regex->match(path);
		if(needInsert)
			_regexCache.insert(extra, regex);

		if(match.hasMatch()) {
			undoSync(); //if it was ever synced, stop syncing it now
			return;
		}
	}

	if(mode == PathSyncHelper::ModeSymlink)
		syncAsSymlink(srcPath(), syncPath());
	else if(mode == PathSyncHelper::ModeCopy)
		syncAsCopy(srcPath(), syncPath());
	else
		fatal("Unsupported path mode");
}

void PathSyncTask::undoSync()
{
	if(mode == PathSyncHelper::ModeSymlink)
		unlink(srcPath(), syncPath());
	else if(mode == PathSyncHelper::ModeCopy)
		SyncTask::undoSync();
	else
		fatal("Unsupported path mode");
}

void PathSyncTask::syncAsSymlink(const QFileInfo &src, const QFileInfo &sync)
{
	// if syncfile does not exist -> move src to sync
	if(!sync.exists()) {
		if(!src.exists())
			return;
		movePath(src, sync, true);
		info() << "Moved file from src to sync folder";
	}

	// check if symlink
	if(src.exists() && src.isSymLink()) {
		//verify it points to dst
		if(src.symLinkTarget() == sync.absoluteFilePath()) {
			debug() << "Symlink is intact";
			return; //all ok
		} else {
			//assume it's because the sync folder change it's location and replace it
			if(!QFile::remove(src.absoluteFilePath()))
				fatal("Failed to remove invalid symlink");
			info() << "Removed invalid symlink";
		}
	}

	// check if exists
	if(src.exists()) {
		Q_ASSERT(!src.isSymLink()); //should be impossible
		if(isFirstUse) {
			if(!removePath(src))
				fatal("Failed to remove src for first sync");
		} else {
			warning() << "Symlink sync conflict - switching over to copy mode";
			syncAsCopy(src, sync); //assume no first use -> copy modified src
			throw NotASymlinkException{};
		}
	}

	//does not exist -> create symlink
	if(!QFile::link(sync.absoluteFilePath(), src.absoluteFilePath()))
		fatal("Failed to create symlink");
	info() << "Created symlink";
}

void PathSyncTask::syncAsCopy(const QFileInfo &src, const QFileInfo &sync)
{
	// dirs are currently not possible as a copy
	if(src.isDir() || sync.isDir())
		fatal("Cannot copy-sync directories! Use symlink sync or wildcard paths");

	// if syncfile does not exist - create it
	if(!sync.exists()) {
		if(!src.exists())
			return;

		if(!QFile::copy(src.absoluteFilePath(), sync.absoluteFilePath()))
			fatal("Failed to copy src to sync");
		info() << "Copied src to sync folder";
		return;
	}

	// check if exists
	if(src.exists()) {
		if(isFirstUse) {
			if(!QFile::remove(src.absoluteFilePath()))
				fatal("Failed to remove src file for first sync");
		} else {
			if(hashFile(src, "src") == hashFile(sync, "sync")) {
				debug() << "Files are the same";
				return; //files are the same
			}

			//else - compare timestamps
			if(src.lastModified() > sync.lastModified()) {
				// remove sync and replace with src
				if(!QFile::remove(sync.absoluteFilePath()))
					fatal("Failed to remove older sync file");
				if(!QFile::copy(src.absoluteFilePath(), sync.absoluteFilePath()))
					fatal("Failed to copy src to sync");
				info() << "Copied src to sync folder";
				return;
			} else { //delete to enter not exist state
				if(!QFile::remove(src.absoluteFilePath()))
					fatal("Failed to remove older src file");
			}
		}
	}

	//does not exist -> copy it
	if(!QFile::copy(sync.absoluteFilePath(), src.absoluteFilePath()))
		fatal("Failed to copy sync to src");
	info() << "Copied sync to src";
}

void PathSyncTask::unlink(const QFileInfo &src, const QFileInfo &sync)
{
	//only do if valid symlink
	if(!src.isSymLink())
		return;
	if(src.symLinkTarget() != sync.absoluteFilePath())
		return;

	if(!QFile::remove(src.absoluteFilePath()))
		fatal("Failed to remove old symlink");

	movePath(sync, src, false);
	info() << "Removed file from synchronisation";
}

void PathSyncTask::movePath(const QFileInfo &from, const QFileInfo &to, bool fromIsSrc)
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
		fatal("Failed to move " + direction + ", because " + fromName +
			  " is a symlink (and symlinks cannot be symlink-synced)");
	}

	if(from.isDir()) { //dir/symlink can only be truly moved, not copy-moved
		QStorageInfo fromInfo{from.absoluteFilePath()};
		QStorageInfo toInfo{to.dir().absolutePath()};

		if(!fromInfo.isValid())
			fatal("Unable to detect volume of " + fromName);
		if(!toInfo.isValid())
			fatal("Unable to detect volume of " + toName);
		if(fromInfo != toInfo)
			fatal("Detected volumes of src and sync are not the same - cannot move directories between different volumes");

		QDir rootDir{fromInfo.rootPath()};
		if(!rootDir.rename(rootDir.relativeFilePath(from.absoluteFilePath()),
						   rootDir.relativeFilePath(to.absoluteFilePath()))) {
			fatal("Failed to directory-move " + direction);
		}
	} else if(!QFile::rename(from.absoluteFilePath(), to.absoluteFilePath()))
		fatal("Failed to move or copy-move " + direction);
}

bool PathSyncTask::removePath(const QFileInfo &path)
{
	if(path.isDir())
		return QDir{path.absoluteFilePath()}.removeRecursively();
	else
		return QFile::remove(path.absoluteFilePath());
}

QByteArray PathSyncTask::hashFile(const QFileInfo &file, const QByteArray &target) const
{
	QCryptographicHash hash(QCryptographicHash::Sha3_512);
	QFile hashFile(file.absoluteFilePath());
	if(!hashFile.open(QIODevice::ReadOnly)) {
		fatal(QStringLiteral("Unable to open %1 file for reading to calculate hashsum with error: %2")
			  .arg(QString::fromUtf8(target), hashFile.errorString()));
	}
	hash.addData(&hashFile);
	return hash.result();
}
