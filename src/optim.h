#ifndef __OPTIM_H__
#define __OPTIM_H__

#include "spectral-api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef char *file_t;

typedef struct 
{
  spectral_value_t *t0;
  spectral_value_t *t1;
} boundaries_t;

Period_t *Analysis (
  signal_t      *signal,
  long long int  t0, 
  long long int  t1, 
  long long int  duration,
  long long int  r0, 
  long long int  r1,
  char          *option, 
  file_t         trace,
  int            cut, 
  signal_t      *signal3,
  int           *num_periods_found, 
  long long int *period, 
  file_t         signalout,
  long long int *point1, 
  long long int *point2, 
  file_t         filename,
  FILE          *out, 
  FILE          *err, 
  int            num_chop,
  int            requested_iters, 
  long long int  sizeTraceOrig,
  long long int  totaltimeTraceOrig);

int GetBoundaries (signal_t *signal, spectral_time_t trace_total_time, spectral_time_t **t0s_out, spectral_time_t **t1s_out, char *dbg_file);

#ifdef __cplusplus
}
#endif

#endif /* __OPTIM_H__ */
