#include "pathsynchelper.h"
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDebug>
#include <QDateTime>
#include <QCryptographicHash>

PathSyncHelper::PathSyncHelper(QObject *parent) :
	QObject(parent),
	_regexCache(100000)
{}

void PathSyncHelper::performSync(const QString &path, SyncEntry::PathMode mode, const QStringList &extras)
{
	for(auto extra : extras) {
		auto regex = _regexCache.object(extra);
		if(!regex) {
			regex = new QRegularExpression(extra,
										   QRegularExpression::DontCaptureOption | QRegularExpression::UseUnicodePropertiesOption);
			regex->optimize();
			_regexCache.insert(extra, regex);
		}

		if(regex->match(path).hasMatch())
			return;
	}

	const static auto home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
	auto basePath = QFileInfo(path).dir().absolutePath();
	auto targetDir = syncDir();
	if(basePath.startsWith(home))
		basePath = QStringLiteral("user/") + QDir(home).relativeFilePath(basePath);
	else
		basePath = QStringLiteral("system/") + basePath;
	targetDir = QDir::cleanPath(targetDir.absoluteFilePath(basePath));

	QFileInfo srcInfo(path);
	srcInfo.setCaching(false);
	QFileInfo syncInfo(targetDir.absoluteFilePath(srcInfo.fileName()));
	syncInfo.setCaching(false);

	//create parent dirs
	if(!srcInfo.dir().exists()) {
		if(!srcInfo.dir().mkpath(QStringLiteral(".")))
			throw SyncException("Failed to create source directory");
	}
	if(!syncInfo.dir().exists()) {
		if(!syncInfo.dir().mkpath(QStringLiteral(".")))
			throw SyncException("Failed to create sync directory");
	}

	switch(mode) {
	case SyncEntry::SymlinkMode:
		syncAsSymlink(srcInfo, syncInfo);
		break;
	case SyncEntry::CopyMode:
		syncAsCopy(srcInfo, syncInfo);
		break;
	default:
		throw SyncException("Unsupported path mode");
	}
}

void PathSyncHelper::syncAsSymlink(const QFileInfo &src, const QFileInfo &sync)
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
			markFirstUsed(src.absoluteFilePath());
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
		if(isFirstUse(src.absoluteFilePath())) {
			if(!QFile::remove(src.absoluteFilePath()))
				throw SyncException("Failed to remove original file for first sync");
		} else {
			qWarning().noquote() << "PATH-SYNC:" << src.absoluteFilePath()
								 << "=> Symlink sync conflict - switching over to copy mode";
			syncAsCopy(src, sync);
			throw NotASymlinkException();
		}
	}

	//does not exist -> create symlink
	if(!QFile::link(sync.absoluteFilePath(), src.absoluteFilePath()))
		throw SyncException("Failed to create symlink");
	log(src, "Created symlink");
	markFirstUsed(src.absoluteFilePath());
}

void PathSyncHelper::syncAsCopy(const QFileInfo &src, const QFileInfo &sync)
{
	// if syncfile does not exist - create it
	if(!sync.exists()) {
		if(!src.exists())
			return;

		if(!QFile::copy(src.absoluteFilePath(), sync.absoluteFilePath()))
			throw SyncException("Failed to copy source to sync");
		log(src, "Copied source to sync folder");
		markFirstUsed(src.absoluteFilePath());
		return;
	}

	// check if exists
	if(src.exists()) {
		if(isFirstUse(src.absoluteFilePath())) {
			if(!QFile::remove(src.absoluteFilePath()))
				throw SyncException("Failed to remove original file for first sync");
		} else {
			if(hashFile(src) == hashFile(sync)) {
				log(src, "Files are the same", true);
				markFirstUsed(src.absoluteFilePath());
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
				markFirstUsed(src.absoluteFilePath());
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
	markFirstUsed(src.absoluteFilePath());
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
