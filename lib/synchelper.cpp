#include "synchelper.h"
#include <QStandardPaths>
#include <QDebug>

namespace {

class SyncErrorException {};

}

SyncHelper::SyncHelper(QObject *parent) :
	QObject(parent)
{}

QString SyncHelper::syncPrefix(const QString &mode) const
{
	return mode;
}

QString SyncHelper::toSyncPath(const QString &mode, const QDir &syncDir, const QString &path) const
{
	auto basePath = QDir::cleanPath(QFileInfo{path}.absoluteFilePath());
	if(basePath.startsWith(QDir::homePath()))
		basePath = syncPrefix(mode) + QStringLiteral("/user/") + QDir::home().relativeFilePath(basePath);
	else
		basePath = syncPrefix(mode) + QStringLiteral("/system/") + basePath;
	return QDir::cleanPath(syncDir.absoluteFilePath(basePath));
}

QString SyncHelper::toSrcPath(const QString &mode, const QDir &syncDir, const QString &path) const
{
	const auto userPrefix = syncDir.absoluteFilePath(syncPrefix(mode) + QStringLiteral("/user/"));
	const auto systemPrefix = syncDir.absoluteFilePath(syncPrefix(mode) + QStringLiteral("/system/"));
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

SyncHelper::ExtrasHint SyncHelper::extrasHint() const
{
	return {false, tr("&Extras"), {}};
}



SyncTask::SyncTask(const SyncHelper *helper, QString mode, const QDir &syncDir, QString path, QStringList extras, bool isFirstUse, QObject *parent) :
	QObject{parent},
	helper{helper},
	mode{std::move(mode)},
	syncDir{syncDir},
	path{std::move(path)},
	extras{std::move(extras)},
	isFirstUse{isFirstUse},
	_isRemove{false}
{
	setAutoDelete(false);
}

SyncTask::SyncTask(const SyncHelper *helper, QString mode, const QDir &syncDir, QString path, QObject *parent)  :
	QObject{parent},
	helper{helper},
	mode{std::move(mode)},
	syncDir{syncDir},
	path{std::move(path)},
	_isRemove{true}
{
	setAutoDelete(false);
}

void SyncTask::run()
{
	try {
		if(_isRemove) {
			undoSync();
			_result = Removed;
		} else {
			performSync();
			_result = Synced;
		}
	} catch(NotASymlinkException &) {
		_result = NotASymlink;
	} catch(SyncErrorException &) {
		_result = Error;
	}

	Q_ASSERT(_result != Invalid);
	QMetaObject::invokeMethod(this, "reportResult", Qt::QueuedConnection);
}

void SyncTask::undoSync()
{
	QFileInfo targetInfo{helper->toSyncPath(mode, syncDir, path)};
	QFile tFile(targetInfo.absoluteFilePath());
	if(tFile.exists()) {
		if(!tFile.remove())
			fatal("Failed to remove synced file");
		info() << "Removed file from synchronisation";
	}
}

QFileInfo SyncTask::srcPath() const
{
	QFileInfo srcInfo{path};
	srcInfo.setCaching(false);
	if(!srcInfo.dir().exists()) {
		if(!srcInfo.dir().mkpath(QStringLiteral(".")))
			fatal("Failed to create source directory");
	}
	return srcInfo;
}

QFileInfo SyncTask::syncPath() const
{
	QFileInfo syncInfo{helper->toSyncPath(mode, syncDir, path)};
	syncInfo.setCaching(false);
	if(!syncInfo.dir().exists()) {
		if(!syncInfo.dir().mkpath(QStringLiteral(".")))
			fatal("Failed to create sync directory");
	}
	return syncInfo;
}

QDebug SyncTask::debug(const QString &extra) const
{
	_log.append({QtDebugMsg, {}});
	QDebug dbg{&(_log.last().second)};
	logBase(dbg, extra);
	return dbg;
}

QDebug SyncTask::info(const QString &extra) const
{
	_log.append({QtInfoMsg, {}});
	QDebug dbg{&(_log.last().second)};
	logBase(dbg, extra);
	return dbg;
}

QDebug SyncTask::warning(const QString &extra) const
{
	_log.append({QtWarningMsg, {}});
	QDebug dbg{&(_log.last().second)};
	logBase(dbg, extra);
	return dbg;
}

QDebug SyncTask::critical(const QString &extra) const
{
	_log.append({QtCriticalMsg, {}});
	QDebug dbg{&(_log.last().second)};
	logBase(dbg, extra);
	return dbg;
}

void SyncTask::fatal(const QString &message, const QString &extra) const
{
	fatal(message.toUtf8(), extra);
}

void SyncTask::fatal(const QByteArray &message, const QString &extra) const
{
	fatal(message.constData(), extra);
}

void SyncTask::fatal(const char *message, const QString &extra) const
{
	critical(extra) << message;
	throw SyncErrorException{};
}

void SyncTask::reportResult()
{
	if(!_log.isEmpty()) {
		auto cat = QStringLiteral("%1-SYNC").arg(mode.toUpper()).toUtf8();
		QMessageLogger logger{nullptr, 0, nullptr, cat.constData()};
		for(const auto &msg : qAsConst(_log)) {
			switch(msg.first) {
			case QtDebugMsg:
				logger.debug("%s", msg.second.toUtf8().constData());
				break;
			case QtInfoMsg:
				logger.info("%s", msg.second.toUtf8().constData());
				break;
			case QtWarningMsg:
				logger.warning("%s", msg.second.toUtf8().constData());
				break;
			case QtCriticalMsg:
				logger.critical("%s", msg.second.toUtf8().constData());
				break;
			default:
				Q_UNREACHABLE();
				break;
			}
		}
	}

	emit syncDone(this, _result);
}

void SyncTask::logBase(QDebug &dbg, const QString &extra) const
{
	QDebugStateSaver state{dbg};
	dbg.nospace().noquote() << path;
	if(!extra.isEmpty())
		dbg << " [" << extra << "]";
	dbg << " =>";
}
