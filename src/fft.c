#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <complex.h>
#include <fftw3.h>
#include "optim-functions.h"
#include "fft.h"

results_fft_t * new_results_fft(int N)
{
  results_fft_t *results_ptr = NULL;

  results_ptr = (results_fft_t *)malloc(sizeof(results_fft_t));
  results_ptr->N    = N;
  results_ptr->freq = (double *)malloc(sizeof(double) * N);
  results_ptr->conj = (double *)malloc(sizeof(double) * N);
  bzero(results_ptr->freq, sizeof(double) * N);
  bzero(results_ptr->conj, sizeof(double) * N);

  return results_ptr;
}

void
fft (results_fftprev_t *input, long long int freq, results_fft_t **fft_results, char *dbg_file)
{
	/**
	*pre:
	*	input: input file
	*	output: output file
	*	freq: freq of sampling
	**/
#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif
  fftw_complex *in  = NULL;
  fftw_complex *out = NULL;
  fftw_plan p;
  results_fft_t *results = NULL;
  int N = 0;

  int i;
#ifdef DEBUG_MODE
  FILE *fd = NULL;
  if (dbg_file != NULL)
  {
    fd = fopen (dbg_file, "w");
    if (fd == NULL)
      {
        fprintf (stderr, "\nfft:: Error: Can't open file '%s'!\n", dbg_file);
        perror("fopen: ");
        exit (-1);
      }
  }
#endif

  N   = input->N;
  in  = input->conj;
  out = fftw_malloc (sizeof (fftw_complex) * N);
  bzero (out, sizeof (fftw_complex) * N);

  p = fftw_plan_dft_1d (N, in, out, -1, FFTW_ESTIMATE);

  fftw_execute (p);

  results = new_results_fft( N );
  for (i = 0; i < N; i++)
  {
    results->freq[i] = 1.0 * i * freq;
    results->conj[i] = pow (cabs (out[i]), 2) / pow (N, 3);
#ifdef DEBUG_MODE
    fprintf (fd, "%lf %lf\n", results->freq[i], results->conj[i]);
#endif
  }

  fftw_destroy_plan (p);
  fftw_free (out);
#ifdef DEBUG_MODE
  fclose (fd);
#endif
#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif

  *fft_results = results;
}
