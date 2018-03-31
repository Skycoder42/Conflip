#include "synchelper.h"
#include <QStandardPaths>

SyncHelper::~SyncHelper() = default;

void SyncHelper::setSyncDir(const QDir &dir)
{
	_syncDir = dir;
	_syncDir.makeAbsolute();
}

QDir SyncHelper::syncDir() const
{
	return _syncDir;
}

std::tuple<QFileInfo, QFileInfo> SyncHelper::generatePaths(const QString &prefix, const QString &path) const
{
	const static auto home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
	auto basePath = QFileInfo(path).dir().absolutePath();
	auto targetDir = syncDir();
	if(basePath.startsWith(home))
		basePath = prefix + QStringLiteral("/user/") + QDir(home).relativeFilePath(basePath);
	else
		basePath = prefix + QStringLiteral("/system/") + basePath;
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

	return {srcInfo, syncInfo};
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
