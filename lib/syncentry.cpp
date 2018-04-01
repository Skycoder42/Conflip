#include "syncentry.h"
#include <QDebug>
#include <QMetaEnum>

bool SyncEntry::operator==(const SyncEntry &other) const
{
	return pathPattern == other.pathPattern &&
			extras == other.extras &&
			mode == other.mode &&
			includeHidden == other.includeHidden &&
			caseSensitive == other.caseSensitive &&
			syncedMachines == other.syncedMachines;
}

bool SyncEntry::operator!=(const SyncEntry &other) const
{
	return pathPattern != other.pathPattern ||
			extras != other.extras ||
			mode != other.mode ||
			includeHidden != other.includeHidden ||
			caseSensitive != other.caseSensitive ||
			syncedMachines != other.syncedMachines;
}

QDebug operator<<(QDebug debug, const SyncEntry &entry)
{
	QDebugStateSaver state(debug);
	if(entry.extras.isEmpty()) {
		debug.noquote().nospace() << entry.pathPattern
								  << ":" << entry.mode;
	} else {
		debug.noquote().nospace() << entry.pathPattern
								  << ":" << entry.mode
								  << "->[" << entry.extras.join(QStringLiteral(", ")) << "]";
	}
	return debug;
}
