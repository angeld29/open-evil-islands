/*
 *  This file is part of Cursed Earth.
 *
 *  Cursed Earth is an open source, cross-platform port of Evil Islands.
 *  Copyright (C) 2009-2010 Yanis Kurganov.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CE_FFT_H
#define CE_FFT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef float FFTSample;

typedef struct FFTComplex {
    FFTSample re, im;
} FFTComplex;

/*
 *  Fast Fourier transform
 *
 *  Based on:
 *  1. FFmpeg (C) Michael Niedermayer
 *            (C) 2008 Loren Merritt
 *            (C) 2002 Fabrice Bellard
 *            partly based on libdjbfft by D. J. Bernstein
*/

typedef struct {
    int nbits;
    int inverse;
    uint16_t *revtab;
    FFTComplex *exptab;
    FFTComplex *exptab1; /* only used by SSE code */
    FFTComplex *tmp_buf;
    int mdct_size; /* size of MDCT (i.e. number of input data * 2) */
    int mdct_bits; /* n = 2^nbits */
    /* pre/post rotation tables */
    FFTSample *tcos;
    FFTSample *tsin;
    int split_radix;
    int permutation;
} FFTContext;

extern int ff_fft_init(FFTContext *s, int nbits, int inverse);
extern void ff_fft_permute_c(FFTContext *s, FFTComplex *z);
extern void ff_fft_calc_c(FFTContext *s, FFTComplex *z);

typedef enum {
    DFT_R2C,
    IDFT_C2R,
    IDFT_R2C,
    DFT_C2R,
} RDFTransformType;

/*
 *  Real Discrete Fourier Transform
*/

typedef struct {
    int nbits;
    int inverse;
    int sign_convention;

    /* pre/post rotation tables */
    const FFTSample *tcos;
    FFTSample *tsin;
    FFTContext fft;
} RDFTContext;

extern int ff_rdft_init(RDFTContext *s, int nbits, RDFTransformType trans);
extern void ff_rdft_end(RDFTContext *s);

extern void ff_rdft_calc_c(RDFTContext *s, FFTSample *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CE_FFT_H */
