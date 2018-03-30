#include "syncentry.h"
#include <QDebug>
#include <QMetaEnum>

bool SyncEntry::operator==(const SyncEntry &other) const
{
	return pathPattern == other.pathPattern &&
			extras == other.extras &&
			mode == other.mode;
}

bool SyncEntry::operator!=(const SyncEntry &other) const
{
	return pathPattern != other.pathPattern ||
			extras != other.extras ||
			mode != other.mode;
}

QDebug operator<<(QDebug debug, const SyncEntry &entry)
{
	QDebugStateSaver state(debug);
	QByteArray modeStr = QMetaEnum::fromType<SyncEntry::PathMode>().valueToKey(entry.mode);
	if(entry.extras.isEmpty()) {
		debug.noquote().nospace() << entry.pathPattern
								  << ":" << modeStr;
	} else {
		debug.noquote().nospace() << entry.pathPattern
								  << ":" << modeStr
								  << "->[" << entry.extras.join(QStringLiteral(", ")) << "]";
	}
	return debug;
}
