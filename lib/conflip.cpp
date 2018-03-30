#include "conflip.h"
#include <QCoreApplication>
#include <QJsonSerializer>
#include <QSet>
#include <QDataStream>
#include "syncentry.h"

namespace {

void conflip_lib_startup()
{
	qRegisterMetaTypeStreamOperators<QSet<QString>>();
	QJsonSerializer::registerAllConverters<SyncEntry>();
}

}

Q_COREAPP_STARTUP_FUNCTION(conflip_lib_startup)
