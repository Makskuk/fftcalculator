/***************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as
** published by the Free Software Foundation, either version 2.1. This
** program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
** FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
** for more details. You should have received a copy of the GNU General
** Public License along with this program. If not, see
** <http://www.gnu.org/licenses/>.
**
***************************************************************************/

#ifndef FFTREAL_WRAPPER_H
#define FFTREAL_WRAPPER_H

#include <QtCore/QtGlobal>
#include "math.h"

#if defined(FFTREAL_LIBRARY)
#  define FFTREAL_EXPORT Q_DECL_EXPORT
#else
#  define FFTREAL_EXPORT Q_DECL_IMPORT
#endif

class FFTRealWrapperPrivate;

// Each pass of the FFT processes 2^X samples, where X is the
// number below.
static const int FFTLengthPowerOfTwo = 11; // 2048 counts for 1024 output numbers

/**
 * Wrapper around the FFTRealFixLen template provided by the FFTReal
 * library
 *
 * This class instantiates a single instance of FFTRealFixLen, using
 * FFTLengthPowerOfTwo as the template parameter.  It then exposes
 * FFTRealFixLen<FFTLengthPowerOfTwo>::do_fft via the calculateFFT
 * function, thereby allowing an application to dynamically link
 * against the FFTReal implementation.
 *
 * See http://ldesoras.free.fr/prod.html
 */
class FFTREAL_EXPORT FFTRealWrapper
{
public:
    FFTRealWrapper();
    ~FFTRealWrapper();

    typedef float DataType;
    void calculateFFT(const DataType in[], DataType out[]);

    static int windowLength() {return pow(2, FFTLengthPowerOfTwo);}

private:
    FFTRealWrapperPrivate*  m_private;
};

#endif // FFTREAL_WRAPPER_H

