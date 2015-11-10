#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <complex.h>
#include <fftw3.h>
#include "optim.h"
#include "optim-macros.h"
#include "optim-functions.h"
#include "dilation.h"
#include "erosion.h"
#include "wavelet.h"
#include "fftprev.h"
#include "fft.h"
#include "spectral-api.h"


/**
 * times_array_out will be a list of pairs of the flushing states timestamps 
 */
int GetValues (signal_t *signal, spectral_time_t **times_array_out)
{
  int count = 0;
  int     j = 0;
  spectral_time_t *times_array = NULL;

#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif

  while (j < SIGNAL_SIZE(signal))
  {
    if (SIGNAL_VALUE(signal, j) != 0)
    {
      times_array = (spectral_time_t *)realloc(times_array, sizeof(spectral_time_t) * (count + 2));

      times_array[count] = SIGNAL_TIME(signal, j);
      count ++;
      times_array[count] = SIGNAL_TIME(signal, j) + SIGNAL_DELTA(signal, j);
      j ++;
      while ((j < SIGNAL_SIZE(signal)) && (SIGNAL_VALUE(signal, j) != 0))
      {
        times_array[count] += SIGNAL_DELTA(signal, j);
        j ++;
      }
      count ++;
    }
    j ++;
  }

  *times_array_out = times_array;

#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif

  return count;
}

int GetBoundaries (signal_t *signal, spectral_time_t trace_total_time, spectral_time_t **t0s_out, spectral_time_t **t1s_out, char *dbg_file)
{
  int      i = 0;
  int count1 = 0;
  int count2 = 0;
  signal_t        *signalD        = NULL;
  signal_t        *signalE        = NULL;
  spectral_time_t *times_pairs    = NULL;
  spectral_time_t *t0s            = NULL;
  spectral_time_t *t1s            = NULL;
  spectral_time_t  total_time_ms  = trace_total_time / 1000000;

  signalD = Dilation(signal,  trace_total_time * 0.001, "dilation.txt");
  signalE = Erosion (signalD, trace_total_time * 0.001, "erosion.txt");
  count1  = GetValues(signalE, &times_pairs);
  
  Spectral_FreeSignal( signalD );
  Spectral_FreeSignal( signalE );
  
#if defined(DEBUG_MODE)
  FILE *fd = fopen(dbg_file, "w");
  for (i = 0; i+1 < count1; i += 2)
  {
    fprintf(fd, FORMAT_TIME" "FORMAT_TIME"\n", times_pairs[i], times_pairs[i+1]);
  }
  fclose(fd);
#endif

  t0s = (spectral_time_t *)malloc( sizeof(spectral_time_t) * count1 + 1);
  t1s = (spectral_time_t *)malloc( sizeof(spectral_time_t) * count1 + 1);

  t0s[0] = 0;
  t1s[0] = total_time_ms;

  for (i = 0; i + 1 < count1; i += 2)
  {
    spectral_time_t a = times_pairs[i]     / 1000000;
    spectral_time_t b = times_pairs[i + 1] / 1000000;

    if (a > 0 && a < total_time_ms)
    {
      t1s[count2] = a;
      if (b < total_time_ms)
      {
              count2 ++;
              t0s[count2] = b;
              t1s[count2] = total_time_ms;
      }
    }
    else if (a <= 0 && b < total_time_ms)
    {
          t0s[count2] = b;
          t1s[count2] = total_time_ms;
    }
    else if (a > total_time_ms)
    {
        t1s[count2] = total_time_ms;
    }
  }

  free(times_pairs);

  *t0s_out = t0s;
  *t1s_out = t1s;

  return count2; 
}



void
GetPeriod (signal_t *signal, long long int Freqx,
	   long long int deltaIN, long long int *T2, int *correct,
	   double *goodness, double *goodness2, double *goodness3,
	   int *zigazaga, int *nzeros)
{
#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif

  /* Correction */
  if (((SIGNAL_LAST_TIME(signal) + SIGNAL_LAST_DELTA(signal)) / 1000000) < deltaIN)
  {
    /* Add one last point to the signal */
    SIGNAL_ADD_POINT_3(
      signal,
      SIGNAL_LAST_TIME(signal) + SIGNAL_LAST_DELTA(signal),
      deltaIN * 1000000 - SIGNAL_LAST_TIME(signal) - SIGNAL_LAST_DELTA(signal),
      0.0
    );
  }

  int num_samples = 0;
  signal_t *sampled = NULL;

  sampled = Sampler (signal, Freqx, "sampler.txt"); 
  num_samples = SIGNAL_SIZE(sampled);


  results_fftprev_t *fftprev_results = NULL;
  results_fft_t     *fft_results     = NULL;

  fftprev (sampled, &fftprev_results, "fftprev.txt");

  fft (fftprev_results, Freqx, &fft_results, "fft.txt");

  GetTime (fft_results, T2, correct, goodness, goodness2, goodness3, zigazaga, nzeros);

  free_results_fftprev( fftprev_results );
  free_results_fft( fft_results );

#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif
}

signal_t *GenerateSinus(long long int span, long long int T2, long long int accuracy, char *dbg_file)
{
  int i = 0, j = 0;
  int sinus_periods = 4;
  int sinus_size    = (span / accuracy) + 2;

  signal_t *sinus = Spectral_AllocateSignal( sinus_size ); 

  for (i = 0; (i * accuracy) < (sinus_periods * T2); i++, j++)
  {
    SIGNAL_ADD_POINT_2(
      sinus,
      2 * (3.1416) / T2 * i * accuracy,
      sin ( 2 * (3.1416) / T2 * i * accuracy ) + 1);
  }
  for (i = (sinus_periods * T2) / accuracy; (i * accuracy) < span; i++, j++) 
  {
    SIGNAL_ADD_POINT_2(
      sinus,
      2 * (3.1416) / T2 * i * accuracy,
      0.0);
  }

#ifdef DEBUG_MODE
  Spectral_DumpSignal( sinus, dbg_file );
#endif

  return sinus;
}



spectral_time_t Maximum(signal_data_t *maximum_point)
{
  return (maximum_point->time+maximum_point->delta/2);
}

Period_t *
Analysis (signal_t *signal1, long long int t0,
	  long long int t1, long long int duration, long long int r0, long long int r1,
	  char *option, file_t trace, int cut, signal_t *signal3,
	  int *pfound, long long int *period, file_t signalout,
	  long long int *point1, long long int *point2, file_t filename, FILE *out,
	  FILE *err, int num_chop, int requested_iters,
	  long long int sizeTraceOrig, long long int totaltimeTraceOrig)
{
#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif
  long long int min, min2, T2, Tcorr, Tprev, requested_final_size;
  long long int size_1_iter, size_1_iter_2, max, accuracy;
  double goodness, goodcorr, goodness2, goodcorr2, goodness3, goodcorr3,
    goodprev, goodprev2, goodprev3;
  int correct, zigazaga, nzeros, zigazagacorr, zigazagaprev, n_iters;
  char *env_var;
  Period_t *currentPeriod = NULL;

  signal_t *signal_cut = Cutter_signal(signal1, t0 * 1000000, t1 * 1000000, NULL, "signal_cut.txt");


  min = t0 * 1000000;
  correct = 0;
  T2 = 1000000;
  Tcorr = -1;
  zigazaga = 0;
  goodness = goodness2 = goodness3 = 0;
  goodcorr = goodcorr2 = goodcorr3 = 0;
  goodprev = goodprev2 = goodprev3 = 0;
  zigazagacorr = zigazagaprev = 0;
  Tprev = 0;

  /* Looking for the main period in signalCutter signal. Condicio correct==0 afegida!! */

  accuracy = 500000;
  while ((T2 / 1000000 != 0 && zigazaga == 0 && T2 / 1000000 < (t1 - t0) / 3.5
	  && correct == 0) || (T2 / 1000000 != 0
			       && T2 / 1000000 < (t1 - t0) / 3.5
			       && (1.0 * (t1 - t0)) / (1.0 * T2 / 1000000) >
			       300))
    {
      //GetPeriod(signalCutter,sizeCutter, accuracy,t1-t0,Tcorrect, tmp_dirname);
      // Accuracy = 5000000 usualment !!!!

      Tprev = T2;
      goodprev = goodness;
      goodprev2 = goodness2;
      goodprev3 = goodness3;
      zigazagaprev = zigazaga;

      GetPeriod (signal_cut, accuracy, t1 - t0, &T2, &correct,
		 &goodness, &goodness2, &goodness3, &zigazaga, &nzeros);

      if (((correct == 1 && goodcorr < goodness)
	   || (zigazaga == 1 && goodcorr < goodness))
	  && (1.0 * (t1 - t0)) / (1.0 * T2 / 1000000) > 2.5)
	{

	  Tcorr = T2;
	  goodcorr = goodness;
	  goodcorr2 = goodness2;
	  goodcorr3 = goodness3;
	  zigazagacorr = zigazaga;
	}
      fprintf (err, "T2=%lld ns\n", T2);
      fprintf (err, "Accuracy=%lld ns\n", accuracy);
      fprintf (err,
	       "Iters=%f, correct=%d, goodness=%lf, goodness2=%lf, goodness3=%lf, zz=%d, nz=%d\n",
	       (1.0 * (t1 - t0)) / (1.0 * T2 / 1000000), correct, goodness,
	       goodness2, goodness3, zigazaga, nzeros);

      accuracy = accuracy + T2 / 10;
    }

  //printf("T2=%lld zigazaga=%d (t1-t0)/2=%ld correct=%d\n", T2, zigazaga, (t1-t0)/2, correct);
  if (Tcorr != -1)
    {
      T2 = Tcorr;
      goodness = goodcorr;
      goodness2 = goodcorr2;
      goodness3 = goodcorr3;
      zigazaga = zigazagacorr;
    }
  else
    {
      T2 = Tprev;
      goodness = goodprev;
      goodness2 = goodprev2;
      goodness3 = goodprev3;
      zigazaga = zigazagaprev;
    }

  fprintf (err, "Main Period=%lld Iters=%f\n", T2 / 1000000,
	   (1.0 * (t1 - t0)) / (1.0 * T2 / 1000000));

  if (T2 > 999999
      &&
      ((goodness > 0.7 && goodness < 1.3 && goodness2 > 0.7
	&& goodness2 < 10.3 && goodness3 < 0.9) || zigazaga == 1) && cut == 1
      && T2 / 1000000 < (t1 - t0) / 3.5)
    {
      fprintf (out,
	       "Region from %lld to %lld ms\nStats:\nStructure:\n   Iters=%f Main Period=%lld ms Likeliness=(%lf,%lf,%lf)\n",
	       r0, r1, (1.0 * (duration)) / (1.0 * T2 / 1000000),
	       T2 / 1000000, goodness, goodness2, goodness3);
      fflush (out);

      /* Save this period into the result vector */
      currentPeriod = (Period_t *) malloc (sizeof (Period_t));
      currentPeriod->iters = (1.0 * (duration)) / (1.0 * T2 / 1000000);
      currentPeriod->length = T2 / 1000000;
      currentPeriod->goodness = goodness;
      currentPeriod->goodness2 = goodness2;
      currentPeriod->goodness3 = goodness3;
      currentPeriod->ini = t0 * 1000000;
      currentPeriod->end = t1 * 1000000;

      *pfound = 1;
      *period = T2 / 1000000;

    }
  else
    {
      /*for(count=0;count<i+1; count++) {printf("   ");}
         printf("No period found\n"); */
      T2 = 0;
    }

  if (trace != NULL)
    {
      sprintf (filename, "%s.%s.chop%d.prv", trace, option, num_chop);

    }

  /*Identifying 2 periods */

  /* Crosscorr */
  //sinus_periods=4
  //N = sinus_periods*T2/acc+span/acc-sinus_periods*T2/acc+2

//  sizeSin =
//    (4 * T2 / 5000000 + ((t1 - t0) * 1000000) / 5000000 - 4 * T2 / 5000000 +
//     2);
//  sinus = (double *) malloc (sizeof (double) * sizeSin);
  // M=(signal[N-1].t+signal[N-1].delta)/freq+1
  

	/**Start Parallel**/
  signal_t *sinus = NULL;
  sinus = GenerateSinus ((t1 - t0) * 1000000, T2, 5000000, "sin.txt");	//para paralelo necesita el sizeSin

/*
  sizeSample =
    (signalCutter[sizeCutter - 1].t +
     signalCutter[sizeCutter - 1].delta) / 5000000 + 1;
  signalSample = (double *) malloc (sizeof (double) * sizeSample);
  Sampler_double (signalCutter, sizeCutter, signalSample, sizeSample,
		  (long long int) 5000000);
*/
  signal_t *sampled_cut = NULL;
  sampled_cut = Sampler(signal_cut, 5000000, "signal_cut_sampled.txt");


//  Crosscorrelation (signalSample, sizeSample, sinus, &min2);
  int x; 
  double y;
  Crosscorrelation (sampled_cut, sinus, "crosscorrelation.txt", &x, &y);
  min2 = x;

	/**End Parallel**/
/*
  free (signalCutter);
  free (sinus);
  free (signalSample);
*/

  Spectral_FreeSignal(signal_cut);
  Spectral_FreeSignal(sampled_cut);
  Spectral_FreeSignal(sinus);



  fprintf (err, "Min accordin to sinus=%lld\n", min + min2 * 5000000);

  /* We cut the original trace in order to provide a 2 periods trace. */

  if (signal3 != NULL)
    {
      signal_data_t maximum_point;

      Cutter_signal(signal3,  min + min2 * 5000000, min + min2 * 5000000 + T2, &maximum_point, "signalt.txt");
      max = Maximum(&maximum_point);
/*
 * max =
	Cutter_signal_Maximum (signal3, sizeSig3, min + min2 * 5000000,
			       min + min2 * 5000000 + T2);
*/
    }
  else
    {
       signal_data_t maximum_point;
       Cutter_signal(signal1, min + min2 * 5000000, min + min2 * 5000000 + T2, &maximum_point, "signalt.txt"); 
       max = Maximum(&maximum_point); 
/*
      max =
	Cutter_signal_Maximum (signal, sizeSig, min + min2 * 5000000,
			       min + min2 * 5000000 + T2);
*/
    }


  /* tall sensible al tamany */
  if (trace != NULL && T2 != 0 && requested_iters == 0)
    {

      printf ("\nCalculating size of 1 iteration...");
      fflush (stdout);
      min2 = min2 / 2;

#if 0
      /* Cut 1 iteration and check the size of the resulting chop */
      spectral_time_t iteration_start_time = 0;
      spectral_time_t iteration_end_time   = 0;

      iteration_start_time = min + min2 * 5000000 + max;
      iteration_end_time   = min + min2 * 5000000 + max + T2;
      size_1_iter   = Cutter3( trace, filename, iteration_start_time, iteration_end_time);

      iteration_start_time = min + min2 * 5000000 + max + 2 * T2;
      iteration_end_time   = min + min2 * 5000000 + max + 3 * T2;
      size_1_iter_2 = Cutter3( trace, filename, iteration_start_time, iteration_end_time);

      if (size_1_iter_2 > size_1_iter)
      {
        size_1_iter = size_1_iter_2;
        max = max + 2 * T2;
      }
#else
      /* This assumes the period is repeated during the whole execution! 
      size_1_iter = sizeTraceOrig / currentPeriod->iters; */

      /* Compute the percentage of time the periodic region represents and use that ratio to approximate the size of a single iteration */
      spectral_time_t periodic_region_length = (currentPeriod->end - currentPeriod->ini);
      double          pct_periodic_region    = ( periodic_region_length * 1.0 / totaltimeTraceOrig * 1.0 ) * 100;
      size_1_iter = (( sizeTraceOrig * pct_periodic_region ) / 100.0) / currentPeriod->iters;
#endif
      fprintf(err, "Size of 1 iteration: %lld\n", size_1_iter);

      printf ("Done!\n");
      fflush (stdout);

      /* We look for the environment var in order to get max trace size */
      if ((env_var = getenv ("SPECTRAL_TRACE_SIZE")) != NULL)
      {
	requested_final_size = atoll (env_var);
      }
      else
      {
	requested_final_size = 100000000LL; /* ~100 Mb */
      }

      max = max + 2 * T2;

      /* How many iterations can we fit in the requested size? */
      if (size_1_iter * 2 > requested_final_size)
      {
        n_iters = 2; /* No less than 2 */
      }
      else
      {
        n_iters = ceil( requested_final_size * 1.0 / size_1_iter * 1.0 ); /* Rounding up */
        /* n_iters = requested_final_size / size_1_iter; */
      }

      /* Is there enough iters in the cutting area? Recalculate the boundaries otherwise */
      if (min + min2 * 5000000 + max + n_iters * T2 > t1 * 1000000)
	{
	  if (n_iters > 2)
	    {

	      while (n_iters * T2 > (t1 - t0) * 1000000 - max)
		{
		  n_iters--;
		}
	      n_iters--;
	      min2 = (t1 - t0) * 1000000 - max - (n_iters * T2);
	      min2 = min2 / 5000000;
	      signal_data_t maximum_point;
              Cutter_signal(signal3, min + min2 * 5000000, min + min2 * 5000000 + T2, &maximum_point, "signalt.txt");
              max = Maximum(&maximum_point);
/*
	      max = 
		Cutter_signal_Maximum (signal3, sizeSig3,
				       min + min2 * 5000000,
				       min + min2 * 5000000 + T2);
*/

	    }
	  else
	    {
	      max = max - 2 * T2;
	    }

	}

    }
  else
  {
    n_iters = MAX (requested_iters, 2);
    n_iters = MIN (n_iters, (int)(currentPeriod->iters));
  }

  /* fi tall sensible al tamany */

  if (T2 > 999999
      &&
      ((goodness > 0.7 && goodness < 1.3 && goodness2 > 0.7
	&& goodness2 < 10.3 && goodness3 < 0.9) || zigazaga == 1) && cut == 1
      && T2 / 1000000 < (t1 - t0) / 3.5)
    {
      if (trace != NULL)
	{
          fprintf(stderr, "Cutting %d iterations for the final trace... \n", n_iters);
	  Cutter2 (trace, filename, min + min2 * 5000000 + max,
		   min + min2 * 5000000 + max + n_iters * T2);
	}
   
      Cutter_signal(signal1, min + min2 * 5000000 + max, min + min2 * 5000000 + max + n_iters * T2, NULL, signalout);


      fprintf (out, "   Detected %d iters between %lld and %lld ns\n",
	       n_iters, min + min2 * 5000000 + max,
	       min + min2 * 5000000 + max + n_iters * T2);

      currentPeriod->best_ini = min + min2 * 5000000 + max;
      currentPeriod->best_end = min + min2 * 5000000 + max + n_iters * T2;

      /* Make sure the recommended iters are inside the region of data */
      while (currentPeriod->best_end > currentPeriod->end)
	{
	  currentPeriod->best_ini -= currentPeriod->length * 1000000;
	  currentPeriod->best_end -= currentPeriod->length * 1000000;
	}
      if (currentPeriod->best_ini < currentPeriod->ini)
	{
	  currentPeriod->best_ini = currentPeriod->ini;
	  fprintf (err, "Detected less iterations than requested!\n");
	}


      *point1 = min / 1000000 + min2 * 5 + max / 1000000;
      *point2 = min / 1000000 + min2 * 5 + max / 1000000 + n_iters * T2 / 1000000;

    }
#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif
  return currentPeriod;
}


