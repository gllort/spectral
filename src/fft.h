#ifndef __FFT_H__
#define __FFT_H__

#include "fftprev.h"

typedef struct {
  int N;
  double *freq;
  double *conj;
} results_fft_t;

#define free_results_fft(ptr)    \
{                                \
  free(ptr->freq);               \
  free(ptr->conj);               \
  ptr->freq = NULL;              \
  ptr->conj = NULL;              \
  ptr->N    = 0;                 \
  free(ptr);                     \
  ptr = NULL;                    \
}

void fft (results_fftprev_t *input, long long int freq, results_fft_t **fft_results, char *dbg_file);

#endif /* __FFT_H__ */
