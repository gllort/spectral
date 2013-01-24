#ifndef _FFT_PREV_H
#define _FFT_PREV_H

#include "spectral-api.h"

typedef struct {
  int N;
  fftw_complex *conj;
} results_fftprev_t;


#define free_results_fftprev(ptr) \
{                                 \
  fftw_free(ptr->conj);           \
  ptr->conj = NULL;               \
  ptr->N    = 0;                  \
  free(ptr);                      \
  ptr = NULL;                     \
}

void fftprev (signal_t *signal, results_fftprev_t **results, char *dbg_file);

#endif /* _FFT_PREV_H */
