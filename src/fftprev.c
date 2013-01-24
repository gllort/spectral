#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>
#include "fftprev.h"
#include <fftw3.h>
#include <strings.h>

#include "spectral-api.h"

results_fftprev_t * new_results_fftprev(int N, fftw_complex *conj)
{
  results_fftprev_t *results_ptr = NULL;

  results_ptr = (results_fftprev_t *)malloc(sizeof(results_fftprev_t));
  results_ptr->N    = N;
  results_ptr->conj = conj;

  return results_ptr;
}


void
fftprev (signal_t *signal, results_fftprev_t **fftprev_results, char *dbg_file)
{
  int i = 0;
  int N = SIGNAL_SIZE(signal);
  fftw_complex *in  = NULL;
  fftw_complex *out = NULL;
  fftw_plan p;
  results_fftprev_t *results = NULL;

#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif

#ifdef DEBUG_MODE
  FILE *fd = NULL;
  if (dbg_file != NULL)
  {
    fd = fopen (dbg_file, "w");
    if (fd == NULL)
    {
      fprintf (stderr, "\nfftprev:: Error: Can't open file '%s'\n", dbg_file);
      perror("fopen: ");
      exit (-1);
    }
  }
#endif

  in   = (fftw_complex *) fftw_malloc (sizeof (fftw_complex) * N);
  out  = (fftw_complex *) fftw_malloc (sizeof (fftw_complex) * N);
  bzero (in,   sizeof (fftw_complex) * N);
  bzero (out,  sizeof (fftw_complex) * N);

  /* Fill the fftw_complex input vector with the signal values */
  for (i = 0; i < N; i ++)
  {
    /* fftw_complex is defined as double[2], this assignment fills the real part only! */
    in[i] = SIGNAL_VALUE(signal, i);
  }

  p = fftw_plan_dft_1d (N, in, out, -1, FFTW_ESTIMATE);

  fftw_execute (p);

  for (i = 0; i < N; i++)
  {
    out[i] = pow (cabs (out[i]), 2) / N;
#ifdef DEBUG_MODE
    fprintf (fd, "%lf\n", out[i]);
#endif
  }
  fftw_free(in);
  fftw_destroy_plan (p);

#ifdef DEBUG_MODE
  fclose (fd);
#endif

#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif

  results = new_results_fftprev(N, out);
  *fftprev_results = results;
}
