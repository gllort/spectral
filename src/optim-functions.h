#ifndef __OPTIM_FUNCTIONS_H__
#define __OPTIM_FUNCTIONS_H__

#define MAXPATHLEN 2048
#define READ_BUFFER_SIZE 8192
#define HW_COUNTER_INSTR 42000050
#define HW_COUNTER_CYC 42000059
#define EVENT_MPIp2p 50000001
#define EVENT_FLUSHING 40000003

#include <complex.h>
#include <fftw3.h>
#include "spectral-api.h"
#include "fft.h"

#define DEFAULT_CHUNK_SIZE 20000000

typedef struct 
{
  long long int time;
  long long int value;
} semantic_t;

void Sampler_wavelet (signal_t *signal, spectral_value_t **waveletSignal, int number_of_points, char *dbg_file);

signal_t * Cutter_signal (signal_t *signal, long long int t0, long long int t1, signal_data_t *maximum_point, char *dbg_file);

void Crosscorrelation(signal_t *signal1, signal_t *signal2, char *dbg_file, int *x, double *y);

long long int Cutter2 (char *input, char *output, long long int from, long long int to);

long long int Cutter3 (char *input, char *output, long long int from, long long int to);

signal_t * Sampler (signal_t *signal, spectral_value_t freq, char *dbg_file);

int qsort_cmp (const void *a, const void *b);

void GetTime(
  results_fft_t *input, 
  long long int *T2,
  int           *correct, 
  double        *goodness, 
  double        *goodness2,
  double        *goodness3, 
  int           *zigazaga, 
  int           *nzeros);



void Trace_Filter_States(char *input_trace, char *filtered_trace, long long int min_time);
void Trace_Filter_Events(char *input_trace, char *filtered_trace, char *filtered_events);

signal_t * Generate_Event_Signal(char *trace, long long int event_type, char *dbg_file);
signal_t * Generate_BW_Signal(char *trace, char *dbg_file);
signal_t * Generate_IPC_Signal(char *trace, char *filtered_trace, char *dbg_file);
signal_t * Generate_MPIp2p_Signal(char *trace, char *filtered_trace, char *dbg_file);
void Generate_Running_Signals (char *input_trace, char *filtered_trace, long long int min_time, signal_t **signal_in_running_out, char *dbg_file_in_running, signal_t **signal_dur_running_out, char *dbg_file_dur_running);



#endif /* __OPTIM_FUNCTIONS_H__ */
