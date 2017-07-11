#ifndef LIBCONFLIP_GLOBAL_H
#define LIBCONFLIP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CONFLIP_LIBRARY)
#  define LIBCONFLIP_EXPORT Q_DECL_EXPORT
#else
#  define LIBCONFLIP_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBCONFLIP_GLOBAL_H
