#ifndef LIB_GLOBAL_H
#define LIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CONFLIP_LIBRARY)
#  define LIBCONFLIPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBCONFLIPSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIB_GLOBAL_H
