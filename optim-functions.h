#ifndef __OPTIM_FUNCTIONS_H__
#define __OPTIM_FUNCTIONS_H__

#define SPECTRAL_HOME(bin) Get_Binary_Dir("SPECTRAL_HOME", bin)
#define PARAVER_HOME(bin)  Get_Binary_Dir("PARAVER_HOME", bin)
#define FILTERS_HOME(bin)  Get_Binary_Dir("FILTERS_HOME", bin)
#define DIMEMAS_HOME(bin)  Get_Binary_Dir("DIMEMAS_HOME", bin)

#define MAXPATHLEN 2048

char * Get_Binary_Dir (char *home_envvar, char *binary);

#define READ_BUFFER_SIZE 8192
#define HW_COUNTER_INSTR 42000050
#define HW_COUNTER_CYC 42000059

struct burst_info {
     long long int time;
     long long int value;
};
                                                                                
struct IPC_info {
	long long int time;
	double value;
};

void Sampler_wavelet(char *input, char *output, int num_of_points);
void Wavelet(char * input, char * output);
void GenerateEventSignal(char *input, char *output, long long int eventType);
void Trace_filter(char *input, char *output, long int type);
void Cutter_signal(char *input, char *output, long long int t0, long long int t1);
void Efficiency(char *input, char *output, int procs);
void Crosscorrelation(char * input1, char * input2, char * output);
void Cutter2(char * input, char * output, long long int a, long long int b);
void Cutter3(char * input, char * output, long long int a, long long int b, int show_messages);
void SenyalD(char * input, long long int k, char * output);
void SenyalE(char * input, long long int k, char * output);
void Sampler(char * input, char * output, long long int Freq);
void Sampler2(char * input, char * output);
void Fft(char * input, char * output, int Freq);
void Fftprev(char * input, char * output, int Freq);
void SignalBW(char * input, char * output);
void SignalRunning(char * input, char * output);
void SignalDurRunning(char * input, char * output);
void SignalDurRunninglog(char * input, char * output);
void SignalIPC(char * input, char * output);
void FilterRunning(char * input, char * output, long long int t);
void Correction(char * Signal, long long int delta, char * Signal2);
void GetTime(char * Freq, char * Signal, int mode, char * Tcorrect, char * output);

int qsort_cmp(const void *a, const void *b);


#endif /* __OPTIM_FUNCTIONS_H__ */
