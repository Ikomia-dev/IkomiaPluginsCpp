#ifndef FACEDETECTOR_GLOBAL_H
#define FACEDETECTOR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(FACEDETECTOR_LIBRARY)
#  define FACEDETECTORSHARED_EXPORT Q_DECL_EXPORT
#else
#  define FACEDETECTORSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // FACEDETECTOR_GLOBAL_H
