#ifndef CONFLIP_H
#define CONFLIP_H

#include <QStringList>
#include <synchelper.h>

#include "lib_conflip_global.h"

namespace Conflip
{

LIBCONFLIPSHARED_EXPORT QStringList listPlugins();
LIBCONFLIPSHARED_EXPORT SyncHelper *loadHelper(const QString &type, QObject *parent = nullptr);

};

#endif // CONFLIP_H
