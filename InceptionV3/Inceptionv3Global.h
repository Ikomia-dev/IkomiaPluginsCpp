#ifndef INCEPTIONV3_GLOBAL_H
#define INCEPTIONV3_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(INCEPTIONV3_LIBRARY)
#  define INCEPTIONV3SHARED_EXPORT Q_DECL_EXPORT
#else
#  define INCEPTIONV3SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // INCEPTIONV3_GLOBAL_H