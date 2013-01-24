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

struct burst_info
{
	long long int time;
	long long int value;
};

struct IPC_info
{
	long long int time;
	double value;
};

struct signal_info
{
	long long int t;
	long long int delta;
	double semanticvalue;
};

struct signalFloat_info
{
	long long int t;
	long long int delta;
	float semanticvalue;
};

struct signalFreq_info
{
	double freq;
	double conj;	
};

#pragma omp task input(signal[*size],size[1],number_of_points) output(waveletSignal[number_of_points])
void Sampler_wavelet(struct signal_info *signal,int *size,double *waveletSignal, int number_of_points);

int Cutter_signal_delta(struct signal_info *signal, int sizeSig,struct signalFloat_info *signal2, long long int t0, long long int t1);
long long int Cutter_signal_Maximum(struct signal_info *signal, int sizeSig,long long int t0, long long int t1);
#pragma omp task input(signal[sizeSig],sizeSig,t0,t1) output(output[1])
void Cutter_signal_OutFile(struct signal_info *signal, int sizeSig,char *output, long long int t0, long long int t1);
#pragma omp task input(signalSample[N],N,sinus[N]) output(min[1])
void Crosscorrelation(double *signalSample,int N,double *sinus,long long int *min);
#pragma omp task input(input[1],a,b) output(output[1])
void Cutter2(char * input, char * output, long long int a, long long int b);

void Sampler_fftw(struct signalFloat_info *signal,int N, fftw_complex *semanticvalue, int M,long long int freq);
#pragma omp task input(signal[N],N,M,freq) output(semanticvalue[M])
void Sampler_double(struct signalFloat_info *signal,int N, double *semanticvalue, int M,long long int freq);

int SignalDurRunning(char * input, struct signal_info *signal);

#pragma omp task input(burst_times[*num_times],num_times[1]) output(signal[*size],size[1])
void signalRunning_out(struct burst_info *burst_times,unsigned long long *num_times,struct signal_info *signal,int *size);
#pragma omp task input(burst_times2[*num_times],num_times[1]) output(signal3[*size3],size3[1])
void signalDurRunning_out(struct burst_info *burst_times2,unsigned long long *num_times,struct signal_info *signal3,int *size3);
#pragma omp task input(ipc_signal[*size]) output(signal[*size]) inout(size[1])
void signalIPC_out(struct IPC_info *ipc_signal,int *size,struct signal_info *signal);
#pragma omp task input(burst_times[*num_times],num_times) output(signal[*size],size[1])
void signalBW_out(struct burst_info *burst_times,unsigned long long *num_times,struct signal_info *signal,int *size);

#pragma omp task input(burst_times3[*num_times2],burst_times4[*num_times2],num_times2[1]) inout(burst_times[*num_times+*num_times2],burst_times2[*num_times+*num_times2],num_times[1])
void merge_burst(struct burst_info *burst_times,struct burst_info *burst_times2,unsigned long long *num_times,struct burst_info *burst_times3,struct burst_info *burst_times4,unsigned long long *num_times2);
#pragma omp task input(burst_times2[*num_times2],num_times2[1]) inout(burst_times[*num_times+*num_times2],num_times[1])
void merge_burst_one(struct burst_info *burst_times,unsigned long long *num_times,struct burst_info *burst_times2,unsigned long long *num_times2);

#pragma omp task input(input[1],total_size,read_point) inout(signal2[40000000],size2[1],last_time[1],signalValue[1])
void get_FlushingSignal(char * input,int total_size,int read_point ,struct signal_info *signal2,int *size2,long long int *last_time,long long int *signalValue);

#pragma omp task input(input[1],total_size,read_point) output(burst_times[40000000],burst_times2[40000000],num_times[1])
void get_Burst_Running_DurRunning(char * input,int total_size,int read_point ,struct burst_info *burst_times,struct burst_info *burst_times2,unsigned long long *num_times);
#pragma omp task input(input[1],total_size,read_point) output(burst_times[40000000],num_times[1])
void get_Burst_DurRunning(char * input,int total_size,int read_point ,struct burst_info *burst_times,unsigned long long *num_times);

int qsort_cmp(const void *a, const void *b);
#pragma omp task input(input[1],total_size,read_point) inout(signal[40000000],size[1],last_time[1],signalValue[1],signal2[40000000],size2[1],last_time2[1],signalValue2[1])
void get_MPIp2p_Flushing_Signal(char * input,int total_size,int read_point ,struct signal_info *signal,int *size,long long int *last_time,long long int *signalValue,struct signal_info *signal2,int *size2,long long int *last_time2,long long int *signalValue2);
#pragma omp task input(input[1],total_size,read_point) inout(ipc_signal[40000000],size[1],signal2[40000000],size2[1],last_time[1],signalValue[1],last_IPC[1000])
void get_IPC_Flushing_Signal(char * input,int total_size,int read_point ,struct IPC_info *ipc_signal,int *size,double *last_IPC,struct signal_info *signal2,int *size2,long long int *last_time,long long int *signalValue);
#pragma omp task input(input[1],total_size,read_point) output(burst_times[40000000],burst_times2[40000000],num_times[1])
void get_Burst_BW_DurRunning(char * input,int total_size,int read_point ,struct burst_info *burst_times,struct burst_info *burst_times2,unsigned long long *num_times);

void FilterRunning(char * input, char * output, long long int t,int option);
void GetTime(struct signalFreq_info *conj,int N,long long int *T2,int *correct, double *goodness, double *goodness2, double *goodness3, int *zigazaga, int *nzeros);
#endif /* __OPTIM_FUNCTIONS_H__ */
