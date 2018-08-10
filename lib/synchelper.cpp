#include "synchelper.h"
#include <QStandardPaths>
#include <QDebug>

SyncHelper::SyncHelper(QObject *parent) :
	QObject(parent)
{}

void SyncHelper::setSyncDir(const QDir &dir)
{
	Q_ASSERT(dir.isAbsolute());
	_syncDir = dir;
}

QString SyncHelper::toSyncPath(const QString &path) const
{
	auto basePath = QDir::cleanPath(QFileInfo{path}.absoluteFilePath());
	if(basePath.startsWith(QDir::homePath()))
		basePath = syncPrefix() + QStringLiteral("/user/") + QDir::home().relativeFilePath(basePath);
	else
		basePath = syncPrefix() + QStringLiteral("/system/") + basePath;
	return QDir::cleanPath(syncDir().absoluteFilePath(basePath));
}

QString SyncHelper::toSrcPath(const QString &path) const
{
	const auto userPrefix = syncDir().absoluteFilePath(syncPrefix() + QStringLiteral("/user/"));
	const auto systemPrefix = syncDir().absoluteFilePath(syncPrefix() + QStringLiteral("/system/"));
	auto basePath = QDir::cleanPath(QFileInfo{path}.absoluteFilePath());
	if(path.startsWith(userPrefix))
		basePath = QDir{userPrefix}.relativeFilePath(basePath);
	else if(path.startsWith(systemPrefix))
		basePath = QDir{systemPrefix}.relativeFilePath(basePath);
	else {
		qWarning() << "Tried to convert path" << path << "from sync to src path, but it is not a synced path";
		return {};
	}
	return QDir::cleanPath(QDir::home().absoluteFilePath(basePath));
}

QDir SyncHelper::syncDir() const
{
	return _syncDir;
}
std::tuple<QFileInfo, QFileInfo> SyncHelper::generatePaths(const QString &path) const
{
	QFileInfo srcInfo{path};
	srcInfo.setCaching(false);
	QFileInfo syncInfo{toSyncPath(path)};
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

	return {srcInfo, syncInfo};
}

void SyncHelper::removeSyncPath(const QString &path, const QByteArray &logPrefix)
{
	QFileInfo targetInfo{toSyncPath(path)};
	QFile tFile(targetInfo.absoluteFilePath());
	if(tFile.exists()) {
		if(!tFile.remove())
			throw SyncException("Failed to remove synced file");
		qInfo().noquote().nospace() << logPrefix << ": " << targetInfo.absoluteFilePath()
									<< " => Removed file from synchronisation";
	}
}



SyncException::SyncException(QByteArray what) :
	QException(),
	_what(std::move(what))
{}

const char *SyncException::what() const noexcept
{
	return _what;
}

void SyncException::raise() const
{
	throw *this;
}

QException *SyncException::clone() const
{
	return new SyncException(_what);
}
