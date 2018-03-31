#include "conflip.h"
#include <QCoreApplication>
#include <QJsonSerializer>
#include "syncentry.h"

namespace {

void conflip_lib_startup()
{
	QJsonSerializer::registerAllConverters<SyncEntry>();
}

}

Q_COREAPP_STARTUP_FUNCTION(conflip_lib_startup)
