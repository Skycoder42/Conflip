#ifndef CONFLIP_H
#define CONFLIP_H

#include <QStringList>
#include <synchelper.h>

#include "lib_conflip_global.h"

namespace Conflip
{

LIBCONFLIPSHARED_EXPORT QString ConfigFileName();
LIBCONFLIPSHARED_EXPORT bool initConfDir();

LIBCONFLIPSHARED_EXPORT QStringList listPlugins();
LIBCONFLIPSHARED_EXPORT void loadTranslations(const QString &type);
LIBCONFLIPSHARED_EXPORT SyncHelper *loadHelper(const QString &type, QObject *parent = nullptr);

};

#endif // CONFLIP_H
