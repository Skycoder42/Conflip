#ifndef LIBCONFLIP_GLOBAL_H
#define LIBCONFLIP_GLOBAL_H

#include <QtCore/qglobal.h>

#include <QUuid>

#if defined(CONFLIP_LIBRARY)
#  define LIBCONFLIP_EXPORT Q_DECL_EXPORT
#else
#  define LIBCONFLIP_EXPORT Q_DECL_IMPORT
#endif

LIBCONFLIP_EXPORT QUuid deviceId();

#endif // LIBCONFLIP_GLOBAL_H
