#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "spectral-api.h"
#include "optim-functions.h"
#include "optim.h"
#include "wavelet.h"

#define _SPECTRAL_MAX(a, b) (a > b ? a : b)

signal_t * Spectral_AllocateSignal(int size)
{
  int max_size = _SPECTRAL_MAX(0, size);
  signal_t *newSignal = NULL;

  newSignal = (signal_t *)malloc(sizeof(signal_t));
  if (newSignal != NULL)
  {
    newSignal->max_size = max_size;
    newSignal->cur_size = 0;
    newSignal->data     = NULL;

    newSignal->data = (signal_data_t *)malloc(sizeof(signal_data_t) * max_size);
    if (newSignal->data != NULL)
    {
      Spectral_ClearSignal(newSignal);
      return newSignal;
    }  
  }
  else 
  {
    Spectral_FreeSignal(newSignal);
  }
  return NULL;
}

void Spectral_ReallocateSignal(signal_t *signal, int new_size)
{
  int i = 0;

  if (new_size > signal->max_size)
  {
    /* Extend the signal */
    signal->data = (signal_data_t *)realloc(signal->data, sizeof(signal_data_t) * new_size);
    /* Initialize the new space to zeros */
    for (i = signal->max_size; i < new_size; i ++)
    {
      signal->data[i].time  = 0;
      signal->data[i].delta = 0;
      signal->data[i].value = 0;
    }
  }
  else if (new_size < signal->max_size)
  {
    /* Set to zeros the data we are going to free */
    for (i = new_size; i < signal->max_size ; i ++)
    {
      signal->data[i].time  = 0;
      signal->data[i].delta = 0;
      signal->data[i].value = 0;
    }
    /* Compress the signal */
    signal->data = (signal_data_t *)realloc(signal->data, sizeof(signal_data_t) * new_size);
  }
  signal->max_size = new_size;
}

void Spectral_ClearSignal(signal_t *signal)
{
  int i = 0;

  if ((signal != NULL) && (signal->data != NULL))
  {
    for (i=0; i<signal->max_size; i++)
    {
      signal->data[i].time  = 0;
      signal->data[i].delta = 0;
      signal->data[i].value = 0;
    }
  }
}

void Spectral_AddPoint2(signal_t *signal, spectral_time_t time, spectral_value_t value)
{
  int idx = SIGNAL_SIZE(signal);


  if ((signal != NULL) && (idx == signal->max_size))
  {
    Spectral_ReallocateSignal(signal, signal->max_size + SIGNAL_CHUNK);
  }

  SIGNAL_TIME(signal, idx) = time;
  SIGNAL_DELTA(signal, idx) = 0;
  if (idx > 0)
  {
    SIGNAL_DELTA(signal, idx-1) = time - SIGNAL_TIME(signal, idx-1);
  } 
  SIGNAL_VALUE(signal, idx) = value;

  SIGNAL_SIZE(signal) ++;
}

void Spectral_AddPoint3(signal_t *signal, spectral_time_t time, spectral_time_t delta, spectral_value_t value)
{
  int idx = SIGNAL_SIZE(signal);

  if ((signal != NULL) && (idx == signal->max_size))
  {
    Spectral_ReallocateSignal(signal, signal->max_size + SIGNAL_CHUNK);
  }

  SIGNAL_TIME(signal, idx)  = time;
  SIGNAL_DELTA(signal, idx) = delta;
  SIGNAL_VALUE(signal, idx) = value;

  SIGNAL_SIZE(signal) ++;
}

signal_t * Spectral_CloneSignal(signal_t *signal_in)
{
  signal_t *signal_out = NULL;

  if (signal_in != NULL)
  {
    int i=0;
    signal_out = Spectral_AllocateSignal(signal_in->cur_size);
    for (i=0; i<signal_in->cur_size; i++)
    {
      signal_out->data[i].time  = signal_in->data[i].time;
      signal_out->data[i].delta = signal_in->data[i].delta;
      signal_out->data[i].value = signal_in->data[i].value;
    }
  }
  return signal_out;
}

int Spectral_CompressSignal(signal_t **signal_in, spectral_time_t min_delta)
{
  int              i, j, k;
  spectral_delta_t delta;
  signal_t        *signal = *signal_in;

  k = 0;
  for (i=0; i<signal->cur_size; i++)
  {
    if (signal->data[i].delta == 0) continue;

    j = i+1;
    delta = signal->data[i].delta;
    while ((j < signal->cur_size) && ((signal->data[j].value == signal->data[i].value) || (signal->data[j].delta == 0) || (signal->data[j].delta < min_delta)))
    {
      delta += signal->data[j].delta;
      j++;
    }
    signal->data[k].time  = signal->data[i].time;
    signal->data[k].delta = delta;
    signal->data[k].value = signal->data[i].value;
    k ++;
    i = j-1;
  }

  signal->cur_size = k;

  Spectral_ReallocateSignal( signal, k );
  *signal_in = signal;
  return k;
}

spectral_time_t Spectral_GetSignalTime(signal_t *signal)
{
  if ((signal != NULL) && (signal->cur_size > 0))
  {
    return SIGNAL_LAST_TIME(signal) + SIGNAL_LAST_DELTA(signal) - SIGNAL_FIRST_TIME(signal);
  }
  else
  {
    return 0;
  }
}

spectral_time_t Spectral_GetSignalSamplingRate(signal_t *signal, int num_samples)
{
  return Spectral_GetSignalTime(signal) / num_samples;
}

int Spectral_DumpSignal(signal_t *signal, char *file)
{
#if defined(DEBUG_MODE)  
  FILE *fd = NULL;

  if ((signal != NULL) && (file != NULL) && ((fd = fopen(file, "w")) != NULL))
  {
    int i = 0;
    for (i = 0; i < signal->cur_size; i ++)
    {
      fprintf(fd, FORMAT_TIME " " FORMAT_DELTA " " FORMAT_VALUE "\n", signal->data[i].time, signal->data[i].delta, signal->data[i].value);
    }
    fclose(fd);
    return 0;
  }
#endif

  return -1;
}

signal_t * Spectral_LoadSignal(char *file)
{
  FILE *fd = NULL;
  char line[LINE_MAX];
  signal_t *loadedSignal = NULL;
  int cur_line = 0;

  if ((fd = fopen(file, "r")) == NULL)
  {
    return NULL;
  }

  cur_line = 0;
  while (fgets(line, LINE_MAX, fd) != NULL)
  {
    cur_line ++;
  }
  rewind(fd);

  loadedSignal = Spectral_AllocateSignal(cur_line);
  
  while (fgets(line, LINE_MAX, fd) != NULL)
  {
    cur_line ++;
    spectral_time_t  time;
    spectral_delta_t delta;
    spectral_value_t value;

    if (sscanf(line, FORMAT_TIME " " FORMAT_DELTA " " FORMAT_VALUE, &time, &delta, &value) == 3)
    { 
      SIGNAL_ADD_POINT_3(
        loadedSignal, 
        time, 
        delta, 
        value
      );
    }
    else
    {
      fprintf(stderr, "Spectral_LoadSignal :: ERROR: Parsing signal from file '%s'\nConflicting line %d: %s\n",
        file, cur_line, line);
      Spectral_FreeSignal(loadedSignal);
      return NULL;
    }
  }
  return loadedSignal;
}

/* Shifts signal to start at 0, otherwise the analysis detects less and less iters as time moves forward */
spectral_time_t Spectral_ShiftSignal(signal_t *signal)
{
  int i = 0;
  spectral_time_t timeShifted = 0;

  if ((signal != NULL) && (signal->cur_size > 0))
  {
    timeShifted = signal->data[0].time;

    if (timeShifted != 0)
    {
      for (i=0; i<signal->cur_size; i++)
      {
        signal->data[i].time -= timeShifted;
      }
    }
  }
  return timeShifted;
}

signal_t * Spectral_ChopSignal (signal_t *signal, spectral_time_t t1, spectral_time_t t2)
{
  int i = 0, t1idx = -1;
  int chop_size;
  signal_t *chopSignal = NULL;

  chop_size = 0;
  for (i=0; i<SIGNAL_SIZE(signal); i++)
  {
    if (signal->data[i].time >= t1)
    {
      if (t1idx == -1) t1idx = i;
      if (signal->data[i].time > t2) break;
      chop_size ++;
    }
  }
  if (chop_size > 0)
  {
    chopSignal = Spectral_AllocateSignal(chop_size);
    memcpy(chopSignal->data, &(signal->data[t1idx]), chop_size * sizeof(signal_data_t));
    SIGNAL_SIZE(chopSignal) = chop_size;
    return chopSignal;
  }
  else
  { 
    return NULL;
  }
}


int Spectral_SerializeSignal (signal_t *signal, spectral_time_t **times_out, spectral_delta_t **deltas_out, spectral_value_t **values_out)
{
  int i = 0;
  spectral_time_t  *times  = NULL;
  spectral_delta_t *deltas = NULL;
  spectral_value_t *values = NULL;

  times  = (spectral_time_t  *)malloc(signal->cur_size * sizeof(spectral_time_t));
  deltas = (spectral_delta_t *)malloc(signal->cur_size * sizeof(spectral_delta_t));
  values = (spectral_value_t *)malloc(signal->cur_size * sizeof(spectral_value_t));

  for (i=0; i<signal->cur_size; i++)
  {
    times[i]  = signal->data[i].time;
    deltas[i] = signal->data[i].delta;
    values[i] = signal->data[i].value;
  }

  *times_out  = times;
  *deltas_out = deltas;
  *values_out = values;

  return signal->cur_size;
}

signal_t * Spectral_AssembleSignal (int size, spectral_time_t *times, spectral_time_t *deltas, spectral_value_t *values)
{
  int i = 0;
  signal_t *signal = NULL;

  if (size > 0)
  {
    signal = Spectral_AllocateSignal(size);
    if (signal != NULL)
    {
      for (i = 0; i < size; i ++)
      {
        Spectral_AddPoint3(signal, times[i], deltas[i], values[i]);
      }

      Spectral_CompressSignal(&signal, 0);
    }
  }

  return signal;
}

#if 0
signal_t * Spectral_AssembleSignal2 (int size, struct burst_info *bursts)
{
  signal_t *signal = NULL;
  if (size > 0)
  {
    signal = Spectral_AllocateSignal(size);
    if (signal != NULL)
    {
      for (i = 0; i < size - 1; i ++)
      {
        if (i > 0) bursts[i].value = bursts[i].value + bursts[i-1].value;
        if (bursts[i+1].time - bursts[i].time > 0)
        {
          signal->data[count].time  = bursts[i].time;
          signal->data[count].delta = bursts[i+1].time - bursts[i].time;
          signal->data[count].value = bursts[i].value;
          count ++;
        }
      }
    }
  }
  signal->data = (signal_data_t *)realloc(signal->data, sizeof(signal_data_t) * count);
  signal->size = count;

  return signal;
}
#endif


double Spectral_CompareSignals(signal_t *signal1_in, signal_t *signal2_in, int windowing)
{
  signal_t *sig1 = NULL, *sig2 = NULL;
  signal_t *sig1_sampled = NULL, *sig2_sampled = NULL;
  spectral_time_t samplingRate = 0;
  int    x1, x2, x3;
  double y1, y2, y3;
  double similarity;  
  char  *dbg_cross_both = NULL;
  char  *dbg_autocross1 = NULL;
  char  *dbg_autocross2 = NULL;
#if defined DEBUG_MODE
  dbg_cross_both = "cross_sig1_sig2.txt";
  dbg_autocross1 = "autocross_sig1.txt";
  dbg_autocross1 = "autocross_sig2.txt";
#endif

  /* XXX Input signals are copied because shifting and windowing modifies them (deleted at exit) */

  sig1 = Spectral_CloneSignal( signal1_in );
  sig2 = Spectral_CloneSignal( signal2_in );

  Spectral_ShiftSignal( sig1 );
  Spectral_ShiftSignal( sig2 );
  
  if (windowing != WINDOWING_NONE)
  {
    applyWindowing( sig1, windowing );
    applyWindowing( sig2, windowing );
  }
#if defined DEBUG_MODE
  Spectral_DumpSignal(sig1, "comp_sig1_shift.txt");
  Spectral_DumpSignal(sig2, "comp_sig2_shift.txt");
#endif

  samplingRate = _SPECTRAL_MAX( 
    Spectral_GetSignalSamplingRate(sig1, 1000),
    Spectral_GetSignalSamplingRate(sig2, 1000));
    
  //sig1_sampled = Sampler(sig1, (spectral_time_t)5000000);
  //sig2_sampled = Sampler(sig2, (spectral_time_t)5000000);
  
  sig1_sampled = Sampler(sig1, (spectral_time_t)samplingRate, NULL);
  sig2_sampled = Sampler(sig2, (spectral_time_t)samplingRate, NULL);

  Crosscorrelation(sig1_sampled, sig1_sampled, dbg_autocross1, &x1, &y1);
  Crosscorrelation(sig2_sampled, sig2_sampled, dbg_autocross2, &x2, &y2);
  Crosscorrelation(sig1_sampled, sig2_sampled, dbg_cross_both, &x3, &y3);

  /* x's -- points where sig1 is more similar to sig2
   * y's -- value of the crosscorrelation 
   */ 
  similarity = sqrt(pow(y3,2)/(y1 * y2));

  Spectral_FreeSignal(sig1_sampled);
  Spectral_FreeSignal(sig2_sampled);
  Spectral_FreeSignal(sig1);
  Spectral_FreeSignal(sig2);

  return similarity;
}

double Spectral_CompareSignalsFromFile(char *signal_file_1, char *signal_file_2, int windowing)
{
  signal_t *sig1, *sig2;

  sig1 = Spectral_LoadSignal(signal_file_1);
  sig2 = Spectral_LoadSignal(signal_file_2);

  return Spectral_CompareSignals(sig1, sig2, windowing);
}

signal_t * Spectral_Add2(signal_t *signal1, signal_t *signal2)
{
  int i = 0, j = 0, k = 0;
  spectral_value_t value1 = 0, value2 = 0;
  signal_t *sumSignal = NULL;

  if ((signal1->cur_size <= 0) && (signal2->cur_size <= 0))
  {
    return NULL;
  }
  else if (signal1->cur_size <= 0)
  {
    return Spectral_CloneSignal(signal2);
  }
  else if (signal2->cur_size <= 0)
  {
    return Spectral_CloneSignal(signal1);
  }

  sumSignal = Spectral_AllocateSignal(signal1->cur_size + signal2->cur_size + 1);
  if (sumSignal != NULL)
  {
    while ((i < signal1->cur_size) || (j < signal2->cur_size))
    {
      if (i >= signal1->cur_size)
      {
        value1 = 0;
        value2 = signal2->data[j].value;
        sumSignal->data[k].time = signal2->data[j].time;
        j++;
      }
      else if (j >= signal2->cur_size)
      {
        value1 = signal1->data[i].value;
        value2 = 0;
        sumSignal->data[k].time = signal1->data[i].time;
        i++;
      }
      else
      {
        if (signal1->data[i].time < signal2->data[j].time)
        {
          value1 = signal1->data[i].value;
          sumSignal->data[k].time = signal1->data[i].time;
          i++;
        }
        else if (signal1->data[i].time > signal2->data[j].time)
        {
          value2 = signal2->data[j].value;
          sumSignal->data[k].time = signal2->data[j].time;
          j++;
        }
        else
        {
          value1 = signal1->data[i].value;
          value2 = signal2->data[j].value;
          sumSignal->data[k].time = signal1->data[i].time;
          i++; j++;
        }
      }
      sumSignal->data[k].value = value1 + value2;
      if (k > 0)
      {
        sumSignal->data[k-1].delta = sumSignal->data[k].time - sumSignal->data[k-1].time;
      }
      k ++;
    }
    /* Last delta */
    if (sumSignal->data[k-1].time == signal1->data[i-1].time)
    {
      sumSignal->data[k-1].delta = signal1->data[i-1].delta;
    }
    else
    {
      sumSignal->data[k-1].delta = signal2->data[j-1].delta;
    }

    sumSignal->cur_size = k;
    Spectral_ReallocateSignal( sumSignal, k );
  }
  return sumSignal;
}


signal_t * Spectral_AddN(int num_signals, signal_t **signals)
{
  int i;
  signal_t *signal1, *signal2, *result;

  if (num_signals <= 0)
  {
    result = NULL;
  }
  else if (num_signals == 1)
  {
    result = Spectral_CloneSignal(signals[0]);
  }
  else
  {
    signal1 = signals[0];
    for (i = 1; i < num_signals; i ++)
    {
      signal2 = signals[i];

      result = Spectral_Add2(signal1, signal2);
      if (i > 1) 
      {
        Spectral_FreeSignal(signal1);
      }
      signal1 = result;
    }
  }
  return result;
}

#if 0
signal_t * Spectral_AddSortedN (int num_signals, signal_t **signals)
{
  int cur_signal = 0;
  int maxAddedSize = 0;
  int count = 0;
  int i = 0;
  struct burst_info *bursts = NULL;
  signal_t *addedSignal = NULL;

  for (cur_signal=0; cur_signal<num_signals; cur_signal++)
  {
    maxAddedSize += signals[i]->size;
  }
  bursts = (struct burst_info *)malloc (maxAddedSize * 2 * sizeof(struct burst_info));
  
  for (cur_signal=0; cur_signal<num_signals; cur_signal++)
  {
    for (i=0; i<signals[cur_signal]->size; i++)
    {
      bursts[count].time  = signals[cur_signal]->data[i].time;
      bursts[count].value = signals[cur_signal]->data[i].value;
      count ++;

      bursts[count].time  = signals[cur_signal]->data[i].time + signals[cur_signal]->data[i].delta; 
      bursts[count].value = -( signals[cur_signal]->data[i].value );
      count ++;
    }
  }
  
  qsort(bursts, count, sizeof(struct burst_info), qsort_cmp);

  addedSignal = Spectral_AssembleSignal2 (count, bursts);
  free(bursts);
  return addedSignal;
}
#endif

static spectral_value_t windowingBarlett(int idx, int size)
{
    spectral_value_t Mid       = (spectral_value_t)(size - 1) / (spectral_value_t)2;
    spectral_value_t K         = ((spectral_value_t)2 / (spectral_value_t)(size - 1));
    spectral_value_t windowing = 0;

    windowing = K * ( Mid - fabs( (spectral_value_t)(idx - Mid) ) );
    return windowing;
}

void applyWindowing(signal_t *signal, int windowing_type)
{
    int n;

    for (n=0; n<signal->cur_size; n++)
    {
        spectral_value_t factor = 1;

        switch(windowing_type)
        {
            case WINDOWING_BARLETT:
                factor = windowingBarlett(n, signal->cur_size);
                break;
            case WINDOWING_10PCT:
                if ( n < signal->cur_size * 0.1) factor = windowingBarlett(n, signal->cur_size);
                break;
            case WINDOWING_NONE:
            default:
                factor = 1;
                break;
        }
        signal->data[n].value = signal->data[n].value * factor;
    }
}

int Spectral_ExecuteAnalysis(signal_t *orig_signal, int target_iters, int windowing_type, Period_t ***DetectedPeriods)
{
  signal_t *signal = NULL;
  spectral_time_t timeShifted = 0, totalTime = 0;
  
  /* Input signal is cloned because shifting (and windowing) modifies it */

  signal = Spectral_CloneSignal( orig_signal );

  timeShifted = Spectral_ShiftSignal( signal );
#if defined(DEBUG_MODE)
  fprintf(stdout, "Signal shifted " FORMAT_TIME " ns\n", timeShifted);
#endif
  totalTime = Spectral_GetSignalTime( signal );

  if (windowing_type != WINDOWING_NONE)
  {
    applyWindowing( signal, windowing_type );
  }

  spectral_value_t *wavelet_samples = NULL;
  Sampler_wavelet(signal, &wavelet_samples, 1024, NULL); /* 1024 or 4096 */

  int sizeSig2;

  Wavelet(wavelet_samples, 1024, &sizeSig2, "wavelet.txt");

  int m = 0;
  int pfound;
  long long int period;
  FILE *out;
  FILE *err;
  long long int t0, t1;
  char *signalout = "/dev/null";
  int numPeriods = 0;
  Period_t **listPeriods = NULL, *foundPeriod = NULL;
 
  while (m < sizeSig2)
  {
    double f1 = wavelet_samples[m];
    double f2 = wavelet_samples[m+1];
    if (f1 < f2)
    {
      spectral_time_t c = signal->data[0].time + totalTime * f1;
      spectral_time_t d = signal->data[0].time + totalTime * f2;

      out = fopen ("report.out", "w");

      err = fopen ("report.err", "w");

      foundPeriod = Analysis(signal, c/1000000, d/1000000, (d-c)/1000000, c/1000000, d/1000000, NULL, NULL, 1, NULL, &pfound, &period, signalout, &t0, &t1, NULL, out, err, 0, target_iters, 0, 0);

      if (foundPeriod != NULL)
      {
        numPeriods ++;
        listPeriods = realloc(listPeriods, numPeriods * sizeof(Period_t *));
        listPeriods[numPeriods - 1] = foundPeriod;
        /* Reapply the time shift to the results */
        foundPeriod->ini += timeShifted;
        foundPeriod->end += timeShifted;
        foundPeriod->best_ini += timeShifted;
        foundPeriod->best_end += timeShifted;
        fprintf(stdout, "PERIOD %d: iters=%f length=%lld g=%lf g2=%lf g3=%lf ini=%lld end=%lld best_ini=%lld best_end=%lld\n",
                                        numPeriods,
                                        foundPeriod->iters,
                                        foundPeriod->length,
                                        foundPeriod->goodness,
                                        foundPeriod->goodness2,
                                        foundPeriod->goodness3,
                                        foundPeriod->ini,
                                        foundPeriod->end,
                                        foundPeriod->best_ini,
                                        foundPeriod->best_end);

      }
      fclose(out);
      fclose(err);
    }
    m += 2;
  }
  fprintf(stdout, "%d period(s) found.\n", numPeriods);

  Spectral_FreeSignal(signal);

  *DetectedPeriods = listPeriods;
  return numPeriods;
}

