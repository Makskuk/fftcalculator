#ifndef FFTREAL_GLOBAL_H
#define FFTREAL_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(FFTREAL_LIBRARY)
#  define FFTREALSHARED_EXPORT Q_DECL_EXPORT
#else
#  define FFTREALSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // FFTREAL_GLOBAL_H
