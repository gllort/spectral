
#ifndef __SIGNAL_INTERFACE_H__
#define __SIGNAL_INTERFACE_H__

#include "optim-functions.h"

/* Function prototypes */
#if defined(__cplusplus)
extern "C" {
#endif

struct signal_elem {
	long long int time;
 	long long int delta;
	long long int value;
};

typedef int signalSize;

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

enum
{
    W_NONE = 0,
    W_BARLETT,
    W_10PCT
};

/* Generates a signal and returns the signal length. The signal is returned in *signal */
signalSize generateSignal(long long int *times, long long int *deltas, long long int *values, signalSize input_size, struct signal_elem **signal);
signalSize generateSignal2 (struct burst_info * burst_times, int count, struct signal_elem **signalOut);


/* Copies a signal */
signalSize copySignal(struct signal_elem *signal_in, signalSize sigsize_in, struct signal_elem **signal_out);

/* Serializes a signal */
void serializeSignal(struct signal_elem *signal, signalSize sigsize, long long int **times_out, long long int **deltas_out, long long int **values_out);

/* Compress signal to eliminate redundant values */
int compressSignal(struct signal_elem **signalOut, signalSize signal_size, long long int min_delta);

/* Destroy the signal */
void destroySignal(struct signal_elem *signal);

/* Dumps signal to disk. Returns 0 on success or -1 if something fails */
int dumpSignal(char *fileName, struct signal_elem *signal, signalSize signal_size);

long long int shiftSignal(struct signal_elem *signal, signalSize size);

double compareSignals(struct signal_elem *sig1, signalSize sig1_len, struct signal_elem *sig2, signalSize sig2_len, int windowing);

double compareFileSignals(char *sig1_txt, char *sig2_txt, int windowing);

void chopSignal (struct signal_elem *signal, signalSize size, long long int t1, long long int t2, struct signal_elem **signal_out, signalSize * size_out);

/* Load signal from file. Returns the signal size or -1 if something fails */
signalSize loadSignal(char *fileName, struct signal_elem **signalIO);

/* Add two signals. Returns signal size on success or 0 if something fails */
signalSize add2Signals(struct signal_elem *signal1, signalSize signal1_size, struct signal_elem *signal2, signalSize signal2_size, struct signal_elem **signalOut);

signalSize addNSignals(struct signal_elem **signals, signalSize *signals_size, int num_signals, struct signal_elem **signalAdded);

signalSize addSorting(int numSignals, struct signal_elem ** listSignals, signalSize * listSizes, struct signal_elem **addedSignal);


int ExecuteAnalysis(struct signal_elem *signal, signalSize size, int num_iters, int windowing, char *outPrefix, Period_t ***ioPeriods);

#if defined(__cplusplus)
}
#endif

#endif /* __SIGNAL_INTERFACE_H__ */
