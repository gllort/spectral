#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>
#include "optim.h"
#include "optim-macros.h"
#include "optim-functions.h"
#include "trace2trace.h"

void Sampler_wavelet (signal_t *signal, spectral_value_t **waveletSignal, int number_of_points, char *dbg_file)
{
#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif
  fprintf (stdout, "Executing Sampler Wavelet...");
  long long int period, last_signal_time;
  spectral_value_t last_value;
  int calculated_periods = 0, i;
  spectral_value_t * wavelet_values = NULL;
#ifdef DEBUG_MODE
  FILE *fp;
  if ((fp = fopen (dbg_file, "w")) == NULL)
    {
      fprintf (stderr, "\nDebug: Can't open file '%s'!\n\n");
      perror("fopen: ");
      exit (-1);
    }
#endif
  wavelet_values = (spectral_value_t *)malloc(sizeof(spectral_value_t) * number_of_points);

  /* Obtaning de last time of the wavelet */
  last_signal_time = SIGNAL_LAST_TIME(signal);

  /* Period = Total time / number of samples */
  period = last_signal_time / number_of_points;

  /* Calculating points */

  last_value = SIGNAL_FIRST_VALUE(signal);
  i = 1;

  while (calculated_periods < number_of_points)
    {
      while (i < SIGNAL_SIZE(signal) && calculated_periods * period > SIGNAL_TIME(signal, i))
	{
	  last_value = SIGNAL_VALUE(signal, i);
	  i++;
	}
      wavelet_values[calculated_periods] = last_value;
      calculated_periods++;
#ifdef DEBUG_MODE
      fprintf (fp, "%lf\n", last_value);
#endif
    }

#ifdef DEBUG_MODE
  fclose (fp);

#endif
  printf ("Done!\n");
  fflush (stdout);

  *waveletSignal = wavelet_values;
#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif
}

signal_t * Cutter_signal (signal_t *signal, long long int t0, long long int t1, signal_data_t *maximum_point, char *dbg_file)
{
  int i = 0;
  signal_t *cut = NULL;
  spectral_time_t  max_time   = 0;
  spectral_time_t  max_delta  = 0;
  spectral_value_t max_value  = 0;

#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif
  cut = Spectral_AllocateSignal( SIGNAL_SIZE(signal) );

  /* Skip times until t0 */
  i = 0;
  while ((i < SIGNAL_SIZE(signal)) && ((SIGNAL_TIME(signal, i) + SIGNAL_DELTA(signal, i)) < t0))
  {
    i++;
  }

  /* Reached t0, add the first point */
  if ((i < SIGNAL_SIZE(signal)) && ((SIGNAL_TIME(signal, i) + SIGNAL_DELTA(signal, i)) >= t0))
  {
    SIGNAL_ADD_POINT_3( cut,
      0,
      SIGNAL_TIME (signal, i) + SIGNAL_DELTA(signal, i) - t0,
      SIGNAL_VALUE(signal, i)
    );
    if (SIGNAL_LAST_VALUE(cut) > max_value)
    {
      max_time  = SIGNAL_LAST_TIME (cut);
      max_delta = SIGNAL_LAST_DELTA(cut);
      max_value = SIGNAL_LAST_VALUE(cut);
    }
    i++;
  }

  /* Between t0 and t1 */
  while ((i < SIGNAL_SIZE(signal)) && ((SIGNAL_TIME(signal, i) + SIGNAL_DELTA(signal, i)) <= t1))
  {
    SIGNAL_ADD_POINT_3( cut,
      SIGNAL_TIME(signal, i) - t0,
      SIGNAL_DELTA(signal, i),
      SIGNAL_VALUE(signal, i)
    );
    if (SIGNAL_LAST_VALUE(cut) > max_value)
    {
      max_time  = SIGNAL_LAST_TIME (cut);
      max_delta = SIGNAL_LAST_DELTA(cut);
      max_value = SIGNAL_LAST_VALUE(cut);
    }
    i++;
  }

  /* Reached t1, add a last point */
  if ((i < SIGNAL_SIZE(signal)) && ((SIGNAL_TIME(signal, i) + SIGNAL_DELTA(signal, i)) > t1))
  {
    SIGNAL_ADD_POINT_3( cut,
      SIGNAL_TIME(signal, i) - t0,
      t1 - SIGNAL_TIME(signal, i),
      SIGNAL_VALUE(signal, i)
    );
    if (SIGNAL_LAST_VALUE(cut) > max_value)
    {
      max_time  = SIGNAL_LAST_TIME (cut);
      max_delta = SIGNAL_LAST_DELTA(cut);
      max_value = SIGNAL_LAST_VALUE(cut);
    }
    i++;
  }

  if (maximum_point != NULL)
  {
    maximum_point->time  = max_time;
    maximum_point->delta = max_delta;
    maximum_point->value = max_value;
  }

  Spectral_ReallocateSignal( cut, SIGNAL_SIZE(cut) );

#ifdef DEBUG_MODE
  Spectral_DumpSignal( cut, dbg_file );
#endif

#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif

  return cut;
}


void Crosscorrelation(signal_t *signal1, signal_t *signal2, char *dbg_file, int *x, double *y)
{
#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif

  fftw_complex *in=NULL, *out=NULL, *in2=NULL, *out2=NULL, *product=NULL;
  fftw_plan p, p2, p3;
  int N=0, i=0, j=0, N1=0, N2=0;
  double *conj1=NULL, *conj2=NULL, max=0;

#ifdef DEBUG_MODE
  FILE *fd;
  fd = fopen (dbg_file, "w");
  if (fd == NULL)
  {
    fprintf (stderr, "\nDebug: Can't open file %s!\n", dbg_file);
    perror("fopen: ");
    exit (-1);
  }
#endif

  N1 = SIGNAL_SIZE(signal1);
  N2 = SIGNAL_SIZE(signal2);

  N = MAX(N1, N2);

  in=fftw_malloc(sizeof(fftw_complex)*N);
  out=fftw_malloc(sizeof(fftw_complex)*N);
  in2=fftw_malloc(sizeof(fftw_complex)*N);
  out2=fftw_malloc(sizeof(fftw_complex)*N);
  product=fftw_malloc(sizeof(fftw_complex)*N);
  conj1=(double *)malloc(sizeof(double)*N);
  conj2=(double *)malloc(sizeof(double)*N);

  bzero(in, sizeof(fftw_complex)*N);
  bzero(out, sizeof(fftw_complex)*N);
  bzero(in2, sizeof(fftw_complex)*N);
  bzero(out2, sizeof(fftw_complex)*N);
  bzero(conj1, sizeof(double)*N);
  bzero(conj2, sizeof(double)*N);

  p=fftw_plan_dft_1d(N, in, out, -1, FFTW_ESTIMATE);
  p2=fftw_plan_dft_1d(N, in2, out2, -1, FFTW_ESTIMATE);

  for(i=0; i<N1; i++) {
    in[i] = signal1->data[i].value;
  }

  for(i=0; i<N2; i++) {
    in2[i] = signal2->data[i].value;
  }

  fftw_execute(p); fftw_execute(p2);


  for(i=0; i<N; i++)  {
          product[i]=out[i]*conj(out2[i]);
//          product2[i]=out2[i]*conj(out[i]);
   }

  p3=fftw_plan_dft_1d(N, product, out, 1, FFTW_ESTIMATE);
  fftw_execute(p3);
//  p4=fftw_plan_dft_1d(N, product2, out2, 1, FFTW_ESTIMATE);
//  fftw_execute(p4);

/*
 *   for(i=0; i<N; i++)  {
 *        fprintf(stderr, "product_out1[%d]=%lf product_out2[%d]=%lf (equal? %d)\n",
 *                i, cabs(out[i]), i, cabs(out2[i]), ( cabs(out[i]) == cabs(out2[i]) ));
 *                  }
 *                  */
  max = 0; j = 0;

  //ULL!! N/p on p nombre d'iteracions, en aquest cas 2

  for(i=0; i<N; i++) {
        conj1[i]=(double)(pow(cabs(out[i]),2)/pow(N,3));

        if(conj1[i]>max) {
                 max=conj1[i];
                 j=i;
        }
  }

#ifdef DEBUG_MODE
  fprintf (fd, "%d %lf\n", j, max);

  for (i = 0; i < N; i++)
    {
      fprintf (fd, "%d %lf\n", i, conj1[i]);
    }
#endif

 
  fftw_destroy_plan (p);
  fftw_destroy_plan (p2);
  fftw_destroy_plan (p3);

  free(in); free(out); free(in2); free(out2); free(product); free(conj1); free(conj2);

#ifdef DEBUG_MODE
  fclose (fd);
#endif

  *x = j;
  *y = max;

#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif

}







long long int Cutter2 (char *input, char *output, long long int from, long long int to)
{
  FILE *fd = NULL;
  char *cutter_args[9];
  char   from_str[128];
  char     to_str[128];
  long long int cut_size_in_bytes = 0;

#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif

  snprintf(from_str, sizeof(from_str), "%lld", from);
  snprintf(to_str,   sizeof(to_str),   "%lld", to);

  cutter_args[0] = "cutter";
  cutter_args[1] = input;
  cutter_args[2] = output;
  cutter_args[3] = "-t";
  cutter_args[4] = from_str;
  cutter_args[5] = to_str;
  cutter_args[6] = "-original_time";
  cutter_args[7] = "-not_break_states";
  cutter_args[8] = "-remove_last_state";

  cutter_main (9, cutter_args);

  fd = fopen(output, "r");
  fseek(fd, 0L, SEEK_END);
  cut_size_in_bytes = ftell( fd );
  fclose(fd);

#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif

  return cut_size_in_bytes;
}

long long int Cutter3 (char *input, char *output, long long int from, long long int to)
{
  FILE *fd = NULL;
  char *cutter_args[6];
  char   from_str[128];  
  char     to_str[128];  
  long long int cut_size_in_bytes = 0;

#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif 

  snprintf(from_str, sizeof(from_str), "%lld", from);
  snprintf(to_str,   sizeof(to_str),   "%lld", to);

  cutter_args[0] = "cutter";
  cutter_args[1] = input;
  cutter_args[2] = output;
  cutter_args[3] = "-t";
  cutter_args[4] = from_str;
  cutter_args[5] = to_str;

  cutter_main (6, cutter_args);

  fd = fopen(output, "r");
  fseek(fd, 0L, SEEK_END);
  cut_size_in_bytes = ftell( fd );
  fclose(fd);

#ifdef TRACE_MODE 
  Extrae_user_function (0);
#endif

  return cut_size_in_bytes;
}

signal_t * Sampler (signal_t *signal, spectral_value_t freq, char *dbg_file)
{
  int i=0, j=0;
  int num_samples = 0;
  signal_t *sampledSignal = NULL;
  spectral_time_t total_time = 0;

#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif

  total_time    = Spectral_GetSignalTime( signal );
  num_samples   = (total_time / freq) + 1;
  sampledSignal = Spectral_AllocateSignal( num_samples );

  while (i < num_samples)
  {
    while ( (i * freq > SIGNAL_TIME(signal, j) + SIGNAL_DELTA(signal, j)) && (j < SIGNAL_SIZE(signal)) )
    {
      j ++;
    }
    SIGNAL_ADD_POINT_3( sampledSignal, 
      signal->data[j].time, 
      signal->data[j].delta, 
      signal->data[j].value 
    );
    i ++;
  }

#ifdef DEBUG_MODE
  Spectral_DumpSignal( sampledSignal, dbg_file );
#endif

#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif

  return sampledSignal;
}


/* Function for use in qsort */
int
qsort_cmp (const void *a, const void *b)
{
  long long int temp =
    ((semantic_t *) a)->time - ((semantic_t *) b)->time;

  /*if(temp < 0) return -1;
     if(temp == 0) return 0;
     if(temp > 0) return 1; */
  if (temp < 0)
    return -1;
  else if (temp > 0)
    return 1;
  else
    return 0;

}


int ParseRunningBurstsFromTrace(char *trace, semantic_t **in_running_out, semantic_t **dur_running_out)
{
  FILE *fd                  = NULL;
  char *buffer              = NULL;
  semantic_t  *in_running = NULL;
  semantic_t *dur_running = NULL;
  int max_bursts = 0;
  int num_bursts = 0;
  int state      = 0;
  long long int t1 = 0, t2 = 0;

  /* Open input file */
  if ((fd = fopen (trace, "r")) == NULL)
  {
    fprintf(stderr, "\nLoadBurstsFromTrace: Can't open file '%s'!\n", trace);
    perror("fopen: ");
    exit (-1);
  }

  buffer = (char *) malloc (READ_BUFFER_SIZE);

  while (fgets (buffer, READ_BUFFER_SIZE, fd) != NULL)
  {
    if (buffer[0] == '1')
    {
      sscanf (buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%lld:%d\n", &t1, &t2, &state);
      if (state == 1)
      {
        if (num_bursts + 1 >= max_bursts)
        {
          /* Allocate more space for bursts */ 
          max_bursts += DEFAULT_CHUNK_SIZE;
          in_running  = (semantic_t *)realloc(in_running,  max_bursts * sizeof(semantic_t));
          dur_running = (semantic_t *)realloc(dur_running, max_bursts * sizeof(semantic_t));
        }
        in_running[num_bursts].time   = t1;
        in_running[num_bursts].value  = 1;
        dur_running[num_bursts].time  = t1;
        dur_running[num_bursts].value = t2 - t1;
        num_bursts++;
        in_running[num_bursts].time   = t2;
        in_running[num_bursts].value  = -1;
        dur_running[num_bursts].time  = t2;
        dur_running[num_bursts].value = -(t2 - t1);
        num_bursts++;
      }
    }
  }

  qsort(in_running,  num_bursts, sizeof(semantic_t), qsort_cmp);
  qsort(dur_running, num_bursts, sizeof(semantic_t), qsort_cmp);

  fclose(fd);
  free(buffer);

  *in_running_out = in_running;
  *dur_running_out = dur_running;
  return num_bursts;
}



void Generate_Running_Signals (char *input_trace, char *filtered_trace, long long int min_time, signal_t **signal_in_running_out, char *dbg_file_in_running, signal_t **signal_dur_running_out, char *dbg_file_dur_running)
{
  int i    = 0;
  semantic_t *bursts_in_running  = NULL;
  semantic_t *bursts_dur_running = NULL;
  int num_bursts        = 0;
  int build_in_running  = (signal_in_running_out  != NULL);
  int build_dur_running = (signal_dur_running_out != NULL);

#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif

  fprintf(stdout, "Generating Running signals... \n");


  Trace_Filter_States(input_trace, filtered_trace, min_time);

  /* Load the trace and parse bursts */
  num_bursts = ParseRunningBurstsFromTrace( ( min_time >= 0 ? filtered_trace : input_trace ), &bursts_in_running, &bursts_dur_running);

  /* Generating signals */
  signal_t *signal_in_running  = NULL;
  signal_t *signal_dur_running = NULL;

  /* In running signal */
  if (build_in_running)
  {
    signal_in_running  = Spectral_AllocateSignal( num_bursts );

    if (bursts_in_running[1].time - bursts_in_running[0].time > 0)
    {
      SIGNAL_ADD_POINT_3( signal_in_running,
        bursts_in_running[0].time,
        bursts_in_running[1].time - bursts_in_running[0].time,
        bursts_in_running[0].value);
    }

    for (i = 1; i < num_bursts - 2; i++)
    {
      bursts_in_running[i].value = bursts_in_running[i].value + bursts_in_running[i - 1].value;
      if (bursts_in_running[i + 1].time - bursts_in_running[i].time > 0)
      {
        SIGNAL_ADD_POINT_3( signal_in_running,
          bursts_in_running[i].time,
          bursts_in_running[i + 1].time - bursts_in_running[i].time,
          bursts_in_running[i].value);
      }
    }
  }

  /* Duration of running bursts signal */
  if (build_dur_running)
  {
    signal_dur_running = Spectral_AllocateSignal( num_bursts );

    if (bursts_dur_running[1].time - bursts_dur_running[0].time > 0)
    {
      SIGNAL_ADD_POINT_3( signal_dur_running,
        bursts_dur_running[0].time,
        bursts_dur_running[1].time - bursts_dur_running[0].time,
        (1.0 * bursts_dur_running[0].value) / 1000000.0
      );
    }

    for (i = 1; i < num_bursts - 2; i++)
    {
      bursts_dur_running[i].value = bursts_dur_running[i].value + bursts_dur_running[i - 1].value;
      if (bursts_dur_running[i + 1].time - bursts_dur_running[i].time > 0)
      {
        SIGNAL_ADD_POINT_3( signal_dur_running,
          bursts_dur_running[i].time,
          bursts_dur_running[i + 1].time - bursts_dur_running[i].time,
          (1.0 * bursts_dur_running[i].value) / 1000000.0
        );
      }
    }
  }

  free (bursts_in_running);
  free (bursts_dur_running);

#if defined(DEBUG_MODE)
  if (build_in_running)  Spectral_DumpSignal(signal_in_running,  dbg_file_in_running);
  if (build_dur_running) Spectral_DumpSignal(signal_dur_running, dbg_file_dur_running);
#endif

#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif

  *signal_in_running_out  = signal_in_running;
  *signal_dur_running_out = signal_dur_running;
}






void
GetTime (results_fft_t *input, long long int *T2,
	 int *correctOut, double *goodness, double *goodness2,
	 double *goodness3, int *zigazagaOut, int *nzeros)
{
#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif

  double p, f, p1, p2, f1, f2, maxp, maxf=0, maxp2, maxf2=0, minp, pant, fant,
    *max_, *max2;
  long long int t, i, j;
  int correct, zigazaga;
  int N = input->N;

  maxp = 0;
  maxp2 = 0;
  minp = 10000000000.0;
  correct = 1;
  max_ = (double *) malloc (sizeof (double) * N);

  //i=0;
  //---> Freqx2.txt ---> fprintf(gp, "%lf %lf\n", 1.0*i*freq, conj[i]);

  f2 = 0;
  p2 = input->conj[0];

  i = 1;
  while ((input->conj[i] == p2) && (i < N / 2 + 3))
    {
      i++;
    }

  fant = input->freq[i];
  pant = input->conj[i];

  p1 = pant;
  f1 = fant;

  /* Provar d'inicialitzar les variables? */

  max2 = max_;

  for (j = i + 1; j < N / 2 + 3; j++)
    {
      f = input->freq[j];
      p = input->conj[j];

      if (p1 != p)
	{
	  if (p2 < p1 && p1 > p)
	    {
	      *max_ = f1;
	      max_++;
	      if (p1 > maxp)
		{
		  maxp = p1;
		  maxf = f1;
		}
	      else if (p1 > maxp2 && p1 < maxp)
		{
		  maxp2 = p1;
		  maxf2 = f1;
		}
	      if (p1 < minp)
		{
		  minp = p1;
		}
	    }

	  p2 = p1;
	  f2 = f1;
	  p1 = p;
	  f1 = f;
	}

    }

  *max_ = -1.0;
  max_ = max2;

  //printf("Period=%lf ms\n", maxf/1000000);

  if ((maxp2 / maxp < 0.9) || (MIN (maxf2, maxf) / MAX (maxf2, maxf) < 0.9)
      || (minp / maxp > 0.99))
    {
      //printf("Warning!!!! -----> Two similar periods\n");
      correct = 0;
    }

  j = 2;
  zigazaga = 1;
  t = maxf;
  i = 0;

#ifdef DEBUG_MODE
  FILE *fDB1;
  fDB1 = fopen ("gettime_output.txt", "a+");
  if (fDB1 == NULL)
    {
      printf ("\nDebug: Can't open file output.txt !!!\n\n");
      exit (-1);
    }
#endif

  while ((i < N) && (j < 5) && (zigazaga == 1))
    {
      f = input->freq[i];
      p = input->conj[i];
#ifdef DEBUG_MODE
      fprintf (fDB1, "%lf %lf\n", 1.0 * j * t, f);
#endif
      if ( /*0.95*f < j*t & j*t < 1.05*f */ f == j * t)
	{
#ifdef DEBUG_MODE
	  fprintf (fDB1, "%lf\n", *max_);
#endif
	  while ((*max_ < f) && (*max_ != -1))
	    {
	      max_++;
	    }

	  if ((0.95 * f < *max_) && (*max_ < 1.05 * f))
	    {
#ifdef DEBUG_MODE
	      fprintf (fDB1, "%lf %lf %lf %lf\n", maxp, maxf, p, f);
#endif

	      j++;
	    }
	  else
	    {
	      zigazaga = 0;
	    }
	}
      i++;
    }

  if (j < 5)
    {
      zigazaga = 0;
    }


  if ((maxf2 / maxf <= 2.05 && maxf2 / maxf >= 1.95)
      || (maxf2 / maxf <= 3.05 && maxf2 / maxf >= 2.95))
    {
      zigazaga = 1;
    }

  //Tcorrect 
  *T2 = t;
  *correctOut = correct;
  *goodness = maxp2 / maxp;
  *goodness2 = maxf2 / maxf;
  *goodness3 = minp / maxp;
  *zigazagaOut = zigazaga;
  *nzeros = j;

#ifdef DEBUG_MODE
  fclose (fDB1);

  fDB1 = fopen ("gettime_Tcorrect", "w");
  if (fDB1 == NULL)
    {
      printf ("\nDebug: Can't open file Tcorrect !!!\n\n");
      exit (-1);
    }
  fprintf (fDB1, "%lld %d %lf %lf %lf %d %lld\n", t, correct, maxp2 / maxp,
	   maxf2 / maxf, minp / maxp, zigazaga, j);
  fclose (fDB1);
#endif

  //free(max_);

#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif

}

signal_t * Generate_Event_Signal(char *trace, long long int event_type, char *dbg_file)
{
  FILE *fileIn;
  long long int type, value, signalValue;
  long long int last_time, current_time;
  char *buffer, events[4096], *token;
  signal_t *signal = NULL;

  /* Alloc buffer for reading file */
  buffer = (char *)malloc(READ_BUFFER_SIZE);

  signal = Spectral_AllocateSignal( SIGNAL_CHUNK );

  /* Parsing trace */
  if((fileIn = fopen(trace, "r")) == NULL)
  {
        fprintf(stderr, "\nGenerateEventSignal:: ERROR: Can't open file '%s'!\n\n", trace);
        perror("fopen: ");
        exit(-1);
  }

  last_time = 0; signalValue = 0;
  while(fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL)
  {
    /* Only events */
    if(buffer[0] == '2')
    {
      sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%s\n", &current_time, events);
      token = strtok(events, ":");
      do 
      {
        /* Obtaining type and value */
        type = atoll(token);
        value = atoll(strtok(NULL, ":"));

        /* Generating Signal */
        if(type == event_type)
        {
          if(current_time != last_time)
          {
            SIGNAL_ADD_POINT_3( signal, 
              last_time,
              current_time - last_time,
              signalValue
            );
            last_time = current_time;
          }
        
          if(value != 0) signalValue++;
          else signalValue--;

          break;
        }
      }
      while((token = strtok(NULL, ":"))!=NULL);
    }
  }
  fclose(fileIn);
  free(buffer);

#if defined(DEBUG_MODE)
  Spectral_DumpSignal( signal, dbg_file );
#endif

  return signal;
}

void Trace_Filter_States(char *input_trace, char *filtered_trace, long long int min_time)
{
  char *trace_filter_args[5];
  char  states[1024];
  char  events[1024];

  snprintf(events, sizeof(events), "-show_events:%d",            40000003);
  snprintf(states, sizeof(states), "-show_states_min_time:%lld", min_time);
  
  trace_filter_args[0] = "trace_filter";
  trace_filter_args[1] = input_trace;
  trace_filter_args[2] = filtered_trace;
  trace_filter_args[3] = events;
  trace_filter_args[4] = states;

  trace_filter_main( 5, trace_filter_args );
}

void Trace_Filter_Events(char *input_trace, char *filtered_trace, char *filtered_events)
{
  char *trace_filter_args[4];
  char events[1024];

  snprintf(events, sizeof(events), "-show_events:%s", filtered_events);

  trace_filter_args[0] = "trace_filter";
  trace_filter_args[1] = input_trace;
  trace_filter_args[2] = filtered_trace;
  trace_filter_args[3] = events;

  trace_filter_main( 4, trace_filter_args );
}

signal_t * Generate_BW_Signal(char *input_trace, char *dbg_file)
{
  FILE         *fd         = NULL;
  char         *buffer     = NULL;
  signal_t     *bw_signal  = NULL;
  semantic_t *comm_times = NULL;
  int           num_comms  = 0;
  int           max_comms  = 0;
  int           i          = 0;

  fprintf(stdout, "Generating BW signal... \n");

  /* Open the trace */
  fd = fopen(input_trace, "r");
  if (fd == NULL)
  {
    fprintf(stderr, "\nGenerate_BW_Signal:: ERROR: Can't open trace '%s'!\n", input_trace);
    perror("fopen: ");
    exit(-1);
  }

  /* Allocate buffers */
  buffer    = (char *)malloc(READ_BUFFER_SIZE);
  bw_signal = Spectral_AllocateSignal( SIGNAL_CHUNK );

  /* Parsing trace */
  while (fgets(buffer, READ_BUFFER_SIZE, fd) != NULL)
  {
    if (buffer[0] == '3')
    {
      long long int time_1 = 0;
      long long int time_2 = 0;
      long long int size   = 0;

      sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%*d:%lld:%*d:%*d:%*d:%*d:%*d:%lld:%lld\n", &time_1, &time_2, &size);

      if (num_comms + 2 > max_comms)
      {
        max_comms += DEFAULT_CHUNK_SIZE;
        comm_times = (semantic_t *)realloc( comm_times, sizeof(semantic_t) * max_comms );
      }

      comm_times[num_comms].time     = time_1;
      comm_times[num_comms + 1].time = time_2;
      if (time_1 != time_2)
      {
        comm_times[num_comms].value     =   size / (time_2 - time_1);
        comm_times[num_comms + 1].value = -(size / (time_2 - time_1));
      }
      else
      {
        comm_times[num_comms].value     = 0;
        comm_times[num_comms + 1].value = 0;
      }
      num_comms += 2;
    }
  }

  /* Sorting */
  qsort(comm_times, num_comms, sizeof(semantic_t), qsort_cmp);

  /* Generating signal XXX Can be generated directly, without using comm_times? */
  bw_signal = Spectral_AllocateSignal (num_comms);

  if (comm_times[0].time > 0)
  {
    SIGNAL_ADD_POINT_3( bw_signal,
      0,
      comm_times[0].time,
      0);
  }

  for (i = 0; i < num_comms - 2; i ++)
  {
    if (comm_times[i+1].time - comm_times[i].time > 0)
    {
      SIGNAL_ADD_POINT_3( bw_signal,
        comm_times[i].time, 
        comm_times[i+1].time - comm_times[i].time, 
        comm_times[i].value);
    }
  }

  fclose(fd);
  free(buffer);
  free(comm_times);

#if defined(DEBUG_MODE)
  Spectral_DumpSignal( bw_signal, dbg_file );
#endif

  return bw_signal;
}

signal_t * Generate_IPC_Signal(char *input_trace, char *filtered_trace, char *dbg_file)
{
  FILE     *fd         = NULL;
  char     *buffer     = NULL;
  signal_t *ipc_signal = NULL;

  fprintf(stdout, "Generating IPC signal... \n");

  /* Invoke trace_filter to filter flushes, instructions and cycles */
  Trace_Filter_Events(input_trace, filtered_trace, "40000003,42000050,42000059");
 
  /* Open the filtered trace */
  fd = fopen(filtered_trace, "r");
  if (fd == NULL)
  {
    fprintf(stderr, "\nGenerate_IPC_Signal:: ERROR: Can't open filtered trace '%s'!\n", filtered_trace);
    perror("fopen: ");
    exit(-1);
  }

  /* Allocate buffers */
  buffer     = (char *)malloc(READ_BUFFER_SIZE);
  ipc_signal = Spectral_AllocateSignal( SIGNAL_CHUNK );

  /* Parsing trace */
  while (fgets(buffer, READ_BUFFER_SIZE, fd) != NULL)
  {
    int           cpu                        = 0;
    long long int current_time               = 0;
    int           instructions_counter_found = 0;
    long long int instructions_counter_value = 0;
    int           cycles_counter_found       = 0;
    long long int cycles_counter_value       = 0;
    char          events[READ_BUFFER_SIZE];
    char         *token                      = NULL;

    instructions_counter_found = 0;
    cycles_counter_found       = 0;

    sscanf(buffer, "%*d:%d:%*d:%*d:%*d:%lld:%s\n", &cpu, &current_time, events);
    token = strtok(events, ":");
    do 
    {
      long long int type  = 0;
      long long int value = 0;

      /* Fetch type and value */
      type  = atoll(token);
      token = strtok(events, ":");
      value = atoll(token);

      if (type == HW_COUNTER_INSTR)
      {
        instructions_counter_found = 1;
        instructions_counter_value = value;
      }
      else if (type == HW_COUNTER_CYC)
      {
        cycles_counter_found = 1;
        cycles_counter_value = value;
      }     
    } while ((token = strtok(NULL, ":")) != NULL);
  
    if (instructions_counter_found && cycles_counter_found)
    { 
      spectral_time_t  time = current_time;
      spectral_value_t ipc  = 0;

      if (cycles_counter_value != 0)
      {
        ipc = (spectral_value_t)instructions_counter_value / (spectral_value_t)cycles_counter_value;
      }

      SIGNAL_ADD_POINT_2( ipc_signal,
        time,
        ipc
      );      
    }
  }

#if defined(DEBUG_MODE)
  Spectral_DumpSignal( ipc_signal, dbg_file );
#endif

  fclose(fd);
  free(buffer);

  return ipc_signal;
}


signal_t * Generate_MPIp2p_Signal(char *input_trace, char *filtered_trace, char *dbg_file)
{
  signal_t *mpip2p_signal = NULL;

  fprintf(stdout, "Generating MPIp2p signal... \n");

  /* Filter the MPI events */
  Trace_Filter_Events(input_trace, filtered_trace, "50000001");

  mpip2p_signal = Generate_Event_Signal(filtered_trace, 50000001, dbg_file);

  return mpip2p_signal;
}


