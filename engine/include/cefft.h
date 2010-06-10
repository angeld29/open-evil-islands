#ifndef AVCODEC_FFT_H
#define AVCODEC_FFT_H

#include <stdint.h>

typedef float FFTSample;

typedef struct FFTComplex {
    FFTSample re, im;
} FFTComplex;

/* FFT computation */
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

/* Real Discrete Fourier Transform */
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

#endif /* AVCODEC_FFT_H */
