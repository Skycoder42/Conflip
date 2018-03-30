#include "synchelper.h"
#include <settings.h>

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

bool SyncHelper::isFirstUse(const QString &path) const
{
	return !static_cast<QSet<QString>>(Settings::instance()->engine.knownEntries).contains(path);
}

void SyncHelper::markFirstUsed(const QString &path)
{
	QSet<QString> set = Settings::instance()->engine.knownEntries;
	if(!set.contains(path)) {
		set.insert(path);
		Settings::instance()->engine.knownEntries = set;
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
