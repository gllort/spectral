#ifndef __SPECTRAL_API_H__
#define __SPECTRAL_API_H__

#include <stdlib.h>

#define SIGNAL_CHUNK 1000

typedef long long int spectral_time_t;
typedef long long int spectral_delta_t;
typedef double        spectral_value_t;

#define FORMAT_TIME  "%lld"
#define FORMAT_DELTA "%lld"
#define FORMAT_VALUE "%lf"

typedef struct
{
  spectral_time_t  time;
  spectral_delta_t delta;
  spectral_value_t value;
} signal_data_t;

typedef struct
{
  int        max_size;
  int        cur_size;
  signal_data_t *data;
} signal_t;

typedef struct
{
  float iters;
  long long int length;
  double goodness;
  double goodness2;
  double goodness3;
  long long int ini;
  long long int end;
  long long int best_ini;
  long long int best_end;
} Period_t;

#define SIGNAL_SIZE(signal)                            signal->cur_size
#define SIGNAL_TIME(signal, i)                         signal->data[i].time
#define SIGNAL_DELTA(signal, i)                        signal->data[i].delta
#define SIGNAL_VALUE(signal, i)                        signal->data[i].value
#define SIGNAL_ADD_POINT_2(signal, time, value)        Spectral_AddPoint2(signal, time, value);
#define SIGNAL_ADD_POINT_3(signal, time, delta, value) Spectral_AddPoint3(signal, time, delta, value);

#define SIGNAL_FIRST_TIME(signal)  SIGNAL_TIME(signal, 0)
#define SIGNAL_FIRST_DELTA(signal) SIGNAL_DELTA(signal, 0)
#define SIGNAL_FIRST_VALUE(signal) SIGNAL_VALUE(signal, 0)
#define SIGNAL_LAST_TIME(signal)   SIGNAL_TIME(signal, SIGNAL_SIZE(signal)-1)
#define SIGNAL_LAST_DELTA(signal)  SIGNAL_DELTA(signal, SIGNAL_SIZE(signal)-1)
#define SIGNAL_LAST_VALUE(signal)  SIGNAL_VALUE(signal, SIGNAL_SIZE(signal)-1)

signal_t * Spectral_AllocateSignal(int size);

void Spectral_ReallocateSignal(signal_t *signal, int new_size);

void Spectral_ClearSignal(signal_t *signal);

#define Spectral_FreeSignal(signal) \
{                                   \
  if (signal != NULL)               \
  {                                 \
    if (signal->data != NULL)       \
    {                               \
      free(signal->data);           \
    }                               \
    signal->data = NULL;            \
    signal->max_size = 0;           \
    signal->cur_size = 0;           \
    free(signal);                   \
    signal = NULL;                  \
  }                                 \
}

void Spectral_AddPoint2(signal_t *signal, spectral_time_t time, spectral_value_t value);

void Spectral_AddPoint3(signal_t *signal, spectral_time_t time, spectral_time_t delta, spectral_value_t value);

signal_t * Spectral_CloneSignal(signal_t *signal_in);

int Spectral_CompressSignal(signal_t **signal_in, spectral_delta_t min_delta);

spectral_time_t Spectral_GetSignalTime(signal_t *signal);

spectral_time_t Spectral_GetSignalSamplingRate(signal_t *signal, int num_samples);

int Spectral_DumpSignal(signal_t *signal, char *file);

signal_t * Spectral_LoadSignal(char *file);

spectral_time_t Spectral_ShiftSignal(signal_t *signal);

signal_t * Spectral_ChopSignal (signal_t *signal, spectral_time_t t1, spectral_time_t t2);

void Spectral_SerializeSignal (signal_t *signal, spectral_time_t **times_out, spectral_delta_t **deltas_out, spectral_value_t **values_out);

signal_t * Spectral_AssembleSignal (int size, long long int *times, long long int *deltas, long long int *values);

//signal_t * Spectral_AssembleSignal2 (int size, struct burst_info *bursts);

double Spectral_CompareSignals(signal_t *signal1_in, signal_t *signal2_in, int windowing);

double Spectral_CompareSignalsFromFile(char *signal_file_1, char *signal_file_2, int windowing);

signal_t * Spectral_Add2(signal_t *signal1, signal_t *signal2);

signal_t * Spectral_AddN(int num_signals, signal_t **signals);

//signal_t * Spectral_AddSortedN (int num_signals, signal_t **signals);

enum
{
    WINDOWING_NONE = 0,
    WINDOWING_BARLETT,
    WINDOWING_10PCT
};

void applyWindowing(signal_t *signal, int windowing_type);

int Spectral_ExecuteAnalysis(signal_t *orig_signal, int target_iters, int windowing_type, Period_t ***DetectedPeriods);

#endif /* __SPECTRAL_API_H__ */

