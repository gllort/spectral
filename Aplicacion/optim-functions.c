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

void Sampler_wavelet(struct signal_info *signal,int *size,double *waveletSignal, int number_of_points)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	printf("Executing Sampler Wavelet..."); fflush(stdout);
	long long int period, last_signal_time;
	char buffer[512];
	double value, last_value;
	int calculated_periods = 0,sizeFinal=0,i;
	#ifdef DEBUG_MODE
		FILE *fp;
		if((fp = fopen("signal.samp.txt", "w")) == NULL)
		{
			printf("\nDebug: Can't open file signal.samp.txt !!!\n\n");
			exit(-1);
		}
	#endif
	/* Obtaning de last time of the wavelet */
	last_signal_time=signal[*size-1].t;

	/* Period = Total time / number of samples */
	period = last_signal_time / number_of_points;

	/* Calculating points */

	last_value=signal[0].semanticvalue;
	i=1;
	
	while ( calculated_periods < number_of_points)
	{
		while(i < *size && calculated_periods * period > signal[i].t)
		{
			last_value = signal[i].semanticvalue;
			i++;
		}
		waveletSignal[calculated_periods]=last_value;
		calculated_periods++;
		#ifdef DEBUG_MODE
			fprintf(fp, "%lf\n", last_value);
		#endif
	}
	
	#ifdef DEBUG_MODE
		fclose(fp);
		
	#endif
	printf("Done!\n"); fflush(stdout);
	
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}


int Cutter_signal_delta(struct signal_info *signal, int sizeSig,struct signalFloat_info *signal2, long long int t0, long long int t1)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	int i,size=0;
	
	/* Cutting signal */
	i=0;
	while(i<sizeSig && ((signal[i].t + signal[i].delta) < t0))
	{
		i++;
	}
	
	if(i<sizeSig && ((signal[i].t + signal[i].delta) >= t0))
	{
		signal2[size].t=0;
		signal2[size].delta=signal[i].t + signal[i].delta - t0;
		signal2[size].semanticvalue=(float)signal[i].semanticvalue;
		size++;
		i++;
	}
	
	while(i<sizeSig && ((signal[i].t + signal[i].delta) <= t1))
	{
		signal2[size].t=signal[i].t - t0;
		signal2[size].delta=signal[i].delta;
		signal2[size].semanticvalue=(float)signal[i].semanticvalue;
		size++;
		i++;
	}
	
	if(i<sizeSig && ((signal[i].t + signal[i].delta) > t1))
	{
		signal2[size].t=signal[i].t - t0;
		signal2[size].delta=t1 - signal[i].t;
		signal2[size].semanticvalue=(float)signal[i].semanticvalue;
		size++;
	}
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
	return size;
}

long long int Cutter_signal_Maximum(struct signal_info *signal, int sizeSig,long long int t0, long long int t1)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	long long int max, maxt, maxdelta;
	int i;
	#ifdef DEBUG_MODE
		FILE *fDB;
		fDB = fopen("signalt2.txt", "w");
		if (fDB==NULL)
		{
			printf("\nDebug: Can't open file signalt2.txt !!!\n\n");
			exit(-1);
		}
	#endif
	max=0;
	maxt=0;
	maxdelta=0;
	
	/* Cutting signal */
	i=0;
	while(i<sizeSig && ((signal[i].t + signal[i].delta) < t0))
	{
		i++;
	}
	
	#ifdef DEBUG_MODE
		if(i<sizeSig && ((signal[i].t + signal[i].delta) >= t0))
		{
			fprintf(fDB, "0 %lld %f\n", signal[i].t + signal[i].delta - t0, (float)signal[i].semanticvalue);
		}
	#endif
	
	if(i<sizeSig && ((signal[i].t + signal[i].delta) >= t0) && (float)signal[i].semanticvalue>max)
	{
		max=(float)signal[i].semanticvalue;
		maxt=0;
		maxdelta=signal[i].t + signal[i].delta - t0;
		i++;
	}
	
	while(i<sizeSig && ((signal[i].t + signal[i].delta) <= t1))
	{
		#ifdef DEBUG_MODE
			fprintf(fDB, "%lld %lld %f\n",signal[i].t - t0, signal[i].delta, (float)signal[i].semanticvalue);
		#endif
		if((float)signal[i].semanticvalue>max)
		{
			max=(float)signal[i].semanticvalue;
			maxt=signal[i].t - t0;
			maxdelta=signal[i].delta;
		}
		i++;
	}
	
	#ifdef DEBUG_MODE
		if(i<sizeSig && ((signal[i].t + signal[i].delta) > t1))
		{
			fprintf(fDB, "%lld %lld %f\n",signal[i].t - t0, t1 - signal[i].t, (float)signal[i].semanticvalue);
		}
	#endif
	
	if(i<sizeSig && ((signal[i].t + signal[i].delta) > t1) && (float)signal[i].semanticvalue>max)
	{
		max=(float)signal[i].semanticvalue;
		maxt=signal[i].t - t0;
		maxdelta=t1 - signal[i].t;		
	}
	
	#ifdef DEBUG_MODE
		fclose(fDB);
	#endif
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
	return (maxt+maxdelta/2);
}

void Cutter_signal_OutFile(struct signal_info *signal, int sizeSig,char *output, long long int t0, long long int t1)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	FILE *fileOut;
	int i;

	/* open output file */
	if((fileOut = fopen(output, "w")) == NULL)
	{
		printf("\nCuttingSignal: Can't open file %s !!!\n\n", output);
		exit(-1);
	}
	
	/* Cutting signal */
	i=0;
	while(i<sizeSig && ((signal[i].t + signal[i].delta) < t0))
	{
		i++;
	}
	
	if(i<sizeSig && ((signal[i].t + signal[i].delta) >= t0))
	{
		fprintf(fileOut, "0 %lld %f\n",signal[i].t + signal[i].delta - t0, (float)signal[i].semanticvalue);
		i++;
	}
	
	while(i<sizeSig && ((signal[i].t + signal[i].delta) <= t1))
	{
		fprintf(fileOut, "%lld %lld %f\n", signal[i].t - t0, signal[i].delta, (float)signal[i].semanticvalue);
		i++;
	}
	
	if(i<sizeSig && ((signal[i].t + signal[i].delta) > t1))
	{
		fprintf(fileOut, "%lld %lld %f\n", signal[i].t - t0, t1 - signal[i].t, (float)signal[i].semanticvalue);
	}
	
	fclose(fileOut);
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}

void Crosscorrelation(double *signalSample,int N,double *sinus,long long int *min)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	fftw_complex *in=NULL, *out=NULL, *in2=NULL, *out2=NULL, *product=NULL;
	fftw_plan p, p2, p3;
	int i, j;
	double *conj1=NULL,max;
	#ifdef DEBUG_MODE
		FILE *fDB1;
		fDB1 = fopen("Crosscorrelation.txt", "w");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file Crosscorrelation.txt !!!\n\n");
			exit(-1);
		}
	#endif
	in=fftw_malloc(sizeof(fftw_complex)*N);
	out=fftw_malloc(sizeof(fftw_complex)*N);
	in2=fftw_malloc(sizeof(fftw_complex)*N);
	out2=fftw_malloc(sizeof(fftw_complex)*N);
	product=fftw_malloc(sizeof(fftw_complex)*N);
	conj1=(double *)malloc(sizeof(double)*N);

	bzero(in, sizeof(fftw_complex)*N);
	bzero(out, sizeof(fftw_complex)*N);
	bzero(in2, sizeof(fftw_complex)*N);
	bzero(out2, sizeof(fftw_complex)*N);
	bzero(conj1, sizeof(double)*N);

	p=fftw_plan_dft_1d(N, in, out, -1, FFTW_ESTIMATE);
	p2=fftw_plan_dft_1d(N, in2, out2, -1, FFTW_ESTIMATE);

	for(i=0; i<N; i++)
	{
		in[i]=signalSample[i];
		in2[i]=sinus[i];
	}

	fftw_execute(p); fftw_execute(p2);

	for(i=0; i<N; i++)
		product[i]=out[i]*conj(out2[i]);

	p3=fftw_plan_dft_1d(N, product, out, 1, FFTW_ESTIMATE);
	fftw_execute(p3);

	max = 0; j = 0;

	//ULL!! N/p on p nombre d'iteracions, en aquest cas 2
	for(i=0; i<N; i++)
	{
		conj1[i]=(double)(pow(cabs(out[i]),2)/pow(N,3));

		if(conj1[i]>max)
		{
			max=conj1[i];
			j=i;
		}
	}
	
	#ifdef DEBUG_MODE
		fprintf(fDB1, "%d %lf\n", j, max);

		for(i=0; i<N; i++)
		{
			fprintf(fDB1, "%d %lf\n", i, conj1[i]);
		}
	#endif
	
	fftw_destroy_plan(p);
	fftw_destroy_plan(p2);
	fftw_destroy_plan(p3);
	fftw_free(in); fftw_free(out); fftw_free(in2); fftw_free(out2); fftw_free(product); free(conj1);
	
	#ifdef DEBUG_MODE
		fclose(fDB1);
	#endif
		
	*min=j;
	
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
	//return j;
	
}


void Cutter2(char * input, char * output, long long int a, long long int b)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	//char aux[2000];
	//sprintf(aux, "%s/cutter %s %s -t %lld %lld -original_time -not_break_states -remove_last_state", FILTERS_HOME("cutter"), input, output, a, b);
	//GS_SYSTEM(aux);
	
	char sa[1000],sb[1000];
	sprintf(sa,"%lld",a);
	sprintf(sb,"%lld",b);
	
	char *aux[9]={"cutter",input,output,"-t",sa,sb,"-original_time","-not_break_states","-remove_last_state"};
	cutter_main(9,aux);
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}

void Sampler_fftw(struct signalFloat_info *signal,int N, fftw_complex *semanticvalue, int M,long long int freq)
{
	/**pre: signal: in data
	*	N: size in data
	*	M: size malloc out data(semanticvalue) which is M=(signal[N-1].t+signal[N-1].delta)/freq+1;
	*	freq: frequency
	* post: semanticvalue: out data
	*	@return real size out data
	**/
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	int i, j;
	#ifdef DEBUG_MODE
		FILE *fDB1;
		fDB1 = fopen("px.txt", "w");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file px.txt !!!\n\n");
			exit(-1);
		}
	#endif
	i=0;
	j=0;
	while(i<M)
	{
		//while( ((long long int)(i*freq) > signal[j].t+signal[j].delta) && j<N )
		while( i*freq > signal[j].t+signal[j].delta && j<N )
		{
			j++;
		}
		semanticvalue[i]=(double)signal[j].semanticvalue;
		#ifdef DEBUG_MODE
			fprintf(fDB1, "%f\n",  signal[j].semanticvalue);
		#endif
		i++;
	}
	
	#ifdef DEBUG_MODE
		fclose(fDB1);
	#endif
		
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}

void Sampler_double(struct signalFloat_info *signal,int N, double *semanticvalue, int M,long long int freq)
{
	/**pre: signal: in data
	*	N: size in data
	*	M: size malloc out data(semanticvalue) which is M=(signal[N-1].t+signal[N-1].delta)/freq+1;
	*	freq: frequency
	* post: semanticvalue: out data
	*	@return real size out data
	**/
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	int i, j;

	i=0;
	j=0;
	while(i<M)
	{
		while(i*freq > signal[j].t+signal[j].delta & j<N)
		{
			j++;
		}
		semanticvalue[i]=signal[j].semanticvalue;

		i++;
	}
	#ifdef DEBUG_MODE
		FILE *fDB1;
		fDB1 = fopen("outin.samp.txt", "w");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file outin.samp.txt !!!\n\n");
			exit(-1);
		}
		j=0;
		while (j<M)
		{
			fprintf(fDB1, "%f\n", semanticvalue[j]);
			j++;
		}
		fclose(fDB1);
	#endif
	//i==size
	//return i;
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}

/* Function for use in qsort */
int qsort_cmp(const void *a, const void *b)
{
	long long int temp = ((struct burst_info *)a)->time - ((struct burst_info *)b)->time;

	/*if(temp < 0) return -1;
	if(temp == 0) return 0;
	if(temp > 0) return 1;*/
	if(temp < 0) return -1;
	else if(temp > 0) return 1;
	else return 0;

}


int SignalDurRunning(char * input, struct signal_info *signal)
{

	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	FILE *fileIn;
	int size=0;

	struct burst_info *burst_times;

	char *buffer;
	unsigned long long num_times = 0, i;
	long long int time_1, time_2;
	int state;

	/* open input file */
	if((fileIn = fopen(input, "r")) == NULL)
	{
		printf("\nGenerateDurRunningSignal: Can't open file %s !!!\n\n", input);
		exit(-1);
	}

	/* Allocating memory for buffers */
	buffer = (char *)malloc(READ_BUFFER_SIZE);
	burst_times = (struct burst_info *)malloc( 20000000 * 2 * sizeof(struct burst_info));

	/* Parsing trace */
	while(fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL)
	{
		if(buffer[0] == '1')
		{
			sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%lld:%d\n", &time_1, &time_2, &state);
			if(state == 1)
			{
				burst_times[num_times].time = time_1;
				burst_times[num_times].value = time_2 - time_1;
				burst_times[num_times+1].time = time_2;
				burst_times[num_times+1].value = -(time_2 - time_1);
				num_times+=2;
			}
		}
	}

	/* Generating signal */

	// QSORT
	qsort(burst_times, num_times, sizeof(struct burst_info), qsort_cmp);

	/* Dumping signal to disk */
	if(burst_times[1].time - burst_times[0].time > 0)
	{
		signal[0].t=burst_times[0].time;
		signal[0].delta=burst_times[1].time - burst_times[0].time;
		signal[0].semanticvalue= (1.0 * burst_times[0].value)/1000000.0;
		size++;
	}

	for(i = 1; i < num_times - 2; i++)
	{
		burst_times[i].value = burst_times[i].value + burst_times[i-1].value;
		if(burst_times[i+1].time - burst_times[i].time > 0)
		{
			signal[size].t=burst_times[i].time;
			signal[size].delta=burst_times[i+1].time - burst_times[i].time;
			signal[size].semanticvalue= (1.0 * burst_times[i].value)/1000000.0;
			size++;
		}
	}

	fclose(fileIn);
	free(buffer); free(burst_times);
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
	return size;
}

void signalRunning_out(struct burst_info *burst_times,unsigned long long *num_times,struct signal_info *signal,int *size)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
/* Generating signalRunning */
	int i;
	*size=0;
	#ifdef TRACE_MODE
			Extrae_event (1000, 13);
		#endif
	qsort(burst_times, *num_times, sizeof(struct burst_info), qsort_cmp);
	#ifdef TRACE_MODE
			Extrae_event (1000, 0);
		#endif
	if(burst_times[1].time - burst_times[0].time > 0)
	{
		signal[0].t=burst_times[0].time;
		signal[0].delta=burst_times[1].time - burst_times[0].time;
		signal[0].semanticvalue= burst_times[0].value;
		*size=1;
	}

	for(i = 1; i < *num_times - 2; i++)
	{
		burst_times[i].value = burst_times[i].value + burst_times[i-1].value;
		if(burst_times[i+1].time - burst_times[i].time > 0)
		{
			signal[*size].t=burst_times[i].time;
			signal[*size].delta=burst_times[i+1].time - burst_times[i].time;
			signal[*size].semanticvalue=burst_times[i].value;
			*size=*size+1;	
		}
	}
	free(burst_times);
	
	#ifdef DEBUG_MODE
		FILE *fDB1;	
		fDB1 = fopen("signal3.txt", "w");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file signal.txt !!!\n\n");
			exit(-1);
		}
		int j=0;
		while (j<*size)
		{
			fprintf(fDB1, "%lld %lld %lf\n", signal[j].t, signal[j].delta, signal[j].semanticvalue);
			j++;
		}
		fclose(fDB1);
	#endif
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}

void signalDurRunning_out(struct burst_info *burst_times2,unsigned long long *num_times,struct signal_info *signal3,int *size3)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	/* Generating signalDurRunning */
	int i;
	*size3=0;
	#ifdef TRACE_MODE
			Extrae_event (1000, 12);
	#endif
	qsort(burst_times2, *num_times, sizeof(struct burst_info), qsort_cmp);
	#ifdef TRACE_MODE
			Extrae_event (1000, 0);
	#endif
	if(burst_times2[1].time - burst_times2[0].time > 0)
	{
		signal3[0].t=burst_times2[0].time;
		signal3[0].delta=burst_times2[1].time - burst_times2[0].time;
		signal3[0].semanticvalue= (1.0 * burst_times2[0].value)/1000000.0;
		*size3=1;
	}

	for(i = 1; i < *num_times - 2; i++)
	{
		burst_times2[i].value = burst_times2[i].value + burst_times2[i-1].value;
		if(burst_times2[i+1].time - burst_times2[i].time > 0)
		{
			signal3[*size3].t=burst_times2[i].time;
			signal3[*size3].delta=burst_times2[i+1].time - burst_times2[i].time;
			signal3[*size3].semanticvalue= (1.0 * burst_times2[i].value)/1000000.0;
			*size3=*size3+1;
		}
	}
	free(burst_times2);
	#ifdef DEBUG_MODE
		FILE *fDB1;	
		fDB1 = fopen("signal.txt", "w");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file signal.txt !!!\n\n");
			exit(-1);
		}
		int j=0;
		while (j<*size3)
		{
			fprintf(fDB1, "%lld %lld %lf\n", signal3[j].t, signal3[j].delta, signal3[j].semanticvalue);
			j++;
		}
		fclose(fDB1);
	#endif
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}

void signalIPC_out(struct IPC_info *ipc_signal,int *size,struct signal_info *signal)
{
	/* Generating signal IPC */
	int i;
	double acumm_ipc = 0;
	
	*size=*size-1;
	
	for(i = 0; i < *size; i++)
	{
		signal[i].t=ipc_signal[i].time;
		signal[i].delta=ipc_signal[i+1].time - ipc_signal[i].time;
		signal[i].semanticvalue=acumm_ipc;
		acumm_ipc += ipc_signal[i].value;
	}
	free(ipc_signal);
}

void signalBW_out(struct burst_info *burst_times,unsigned long long *num_times,struct signal_info *signal,int *size)
{
	int i;
	*size=0;
	/* Generating signalBW */
		
	qsort(burst_times, *num_times, sizeof(struct burst_info), qsort_cmp);
	
	if(burst_times[0].time > 0)
	{
		signal[0].t=0;
		signal[0].delta=burst_times[0].time;
		signal[0].semanticvalue=0;
		*size=1;
	}

	for(i = 0; i < *num_times - 2; i++)
	{
		if(burst_times[i+1].time - burst_times[i].time > 0)
		{
			signal[*size].t=burst_times[i].time;
			signal[*size].delta=burst_times[i+1].time - burst_times[i].time;
			signal[*size].semanticvalue=burst_times[i].value;
			*size=*size+1;
		}
	}
	free(burst_times);
}

void merge_burst(struct burst_info *burst_times,struct burst_info *burst_times2,unsigned long long *num_times,struct burst_info *burst_times3,struct burst_info *burst_times4,unsigned long long *num_times2)
{
	int i,j;
	if(burst_times!=burst_times3 && *num_times2!=0)
	{
		j=*num_times;
		for(i=0;i<*num_times2;i++)
		{
			burst_times[j]=burst_times3[i];
			burst_times2[j]=burst_times4[i];
			j++;
		}
		*num_times += *num_times2;
	}
}

void merge_burst_one(struct burst_info *burst_times,unsigned long long *num_times,struct burst_info *burst_times2,unsigned long long *num_times2)
{
	int i,j;
	if(burst_times!=burst_times2 && *num_times2!=0)
	{
		j=*num_times;
		for(i=0;i<*num_times2;i++)
		{
			burst_times[j]=burst_times2[i];
			j++;
		}
		*num_times += *num_times2;
	}
}

#if 0
void GenerateSemanticsSignals(char * input, struct signal_info *signal,int *size,struct signal_info *signal2,int *size2,struct signal_info *signal3,int *size3, int option)
{
	/**pre: 
	 * 	-input: traza para generar las señales semanticas
	 * 	-signal,signal2,signal3: struct de las señales semanticas para out
	 * 	-size,size2,size3: tamaño de las señales generadas para out
	 * 	-option: opcion para que señal generar en signal segun la opcion pasada por usuario
	 * 		-1: BW
	 * 		-2: IPC
	 * 		-3: MPIp2p
	 * 		-4: CPUBurst
	 * 		-5: CPUDurBurst
	 * post: @return generar las señales semanticas segun la option:
	 * 	-1: BW --> signal contendra la señal de BW (SignalBW) con el tipo communicaciones
	 * 	-2: IPC --> signal contendra la señal de IPC (SignalIPC) con tipo evento HW_COUNTER_CYC y HW_COUNTER_INSTR
	 * 	-3: MPIp2p --> signal contendra la señal de evento (GenerateEventSignal) con tipo evento EVENT_MPIp2p
	 * 	-4: CPUBurst --> signal contendra la señal de Running (signalRunning) con tipo state
	 * 	-5: CPUDurBurst --> signal contendra la señal de Running (signalRunning) con tipo state
	 * 	
	 * 	para todas las opciones en signal2 se generara la señal de eventos (GenerateEventSignal) con tipo evento EVENT_FLUSHING
	 * 	y en signal3 se generara la señal de DurRunning (signalDurRunning) con tipo state
	 **/
	
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	FILE *fileIn;
	*size=0;
	*size2=0;
	*size3=0;
	struct burst_info *burst_times,*burst_times2;

	char *buffer;
	unsigned long long num_times = 0, i;
	long long int time_1, time_2,size_comm;
	int state;
	
	long long int type, value, signalValue,signalValue2;
	long long int last_time,last_time2, current_time;
	char events[4096], *token;
	
	
	struct IPC_info *ipc_signal;
	long long int instructions, cycles;
	int counter_found, cpu;
	double *last_IPC;
		
	/* open input file */
	if((fileIn = fopen(input, "r")) == NULL)
	{
		printf("SignalRunning_DurRunning_GenerateEventSignal: Can't open file %s !!!\n\n", input);
		exit(-1);
	}

	/* Allocating memory for buffers */
	buffer = (char *)malloc(READ_BUFFER_SIZE);
	if(option==5 || option==4 || option==1)
	{
		//para signalRunning option 4 y 5
		//para SignalBW option 1
		burst_times = (struct burst_info *)malloc( SIZE_SIGNAL* sizeof(struct burst_info));
	}
	else if(option == 2)
	{
		ipc_signal = (struct IPC_info *)malloc(SIZE_SIGNAL_IPC * sizeof(struct IPC_info));
		last_IPC = (double *)malloc(1000 * sizeof(double));
	}
	
	burst_times2 = (struct burst_info *)malloc(SIZE_SIGNAL * sizeof(struct burst_info));//signalDurRunning
	
	//vars GenerateEventSignal 40000003
	last_time = 0; signalValue = 0;
	
	//vars GenerateEventSignal eventType
	last_time2 = 0; signalValue2 = 0;
	
	/* Parsing trace */
	while(fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL)
	{
		if(buffer[0] == '1')/* states */
		{
			sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%lld:%d\n", &time_1, &time_2, &state);
			if(state == 1)
			{
				if(option==5 || option==4)
				{
					//signalRunning
					burst_times[num_times].time = time_1;
					burst_times[num_times].value = 1;
					burst_times[num_times+1].time = time_2;
					burst_times[num_times+1].value = -1;
				}
				
				//signalDurRunning
				burst_times2[num_times].time = time_1;
				burst_times2[num_times].value = time_2 - time_1;
				burst_times2[num_times+1].time = time_2;
				burst_times2[num_times+1].value = -(time_2 - time_1);
				num_times+=2;
			}
		}
		else if(buffer[0] == '2')/* events */
		{
			//GenerateEventSignal
			
			//sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%s\n", &current_time, events);
			sscanf(buffer, "%*d:%d:%*d:%*d:%*d:%lld:%s\n", &cpu, &current_time, events);
			token = strtok(events, ":");
			counter_found =0;
			do
			{
				/* Obtaining type and value */
				type = atoll(token);
				value = atoll(strtok(NULL, ":"));

				/* Generating Signal */
				if(type == EVENT_FLUSHING)//EventSignal for flushing
				{
					if(current_time != last_time)
					{
						signal2[*size2].t=last_time;
						signal2[*size2].delta=current_time - last_time;
						signal2[*size2].semanticvalue=signalValue;
						last_time = current_time;
						*size2 = *size2 + 1;
					}

					if(value != 0) signalValue++;
					else signalValue--;
				}
				else if(option == 3 && type == EVENT_MPIp2p)//EventSignal for MPIp2p
				{
					if(current_time != last_time2)
					{
						signal[*size].t=last_time2;
						signal[*size].delta=current_time - last_time2;
						signal[*size].semanticvalue=signalValue2;
						last_time2 = current_time;
						
						*size = *size + 1;
						
					}
					if(value != 0) signalValue2++;
					else signalValue2--;
					
				}
				else if(option == 2 && type == HW_COUNTER_INSTR)
				{
					counter_found = 1;
					instructions = value;
				}
				else if(option == 2 && type == HW_COUNTER_CYC)
				{
					counter_found = 1;
					cycles = value;
				}
			}
			while((token = strtok(NULL, ":"))!=NULL);
			
			if(option == 2 && counter_found)
			{
				ipc_signal[*size].time = current_time;
				if(cycles != 0)
					ipc_signal[*size].value = ((double)instructions/(double)cycles) - last_IPC[cpu];
				else
					ipc_signal[*size].value = 0.0;

				last_IPC[cpu] = ipc_signal[*size].value;
				*size=*size+1;
			}
		}
		else if(buffer[0] == '3' && option == 1) /* comm */
		{
			sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%*lld:%lld:%*d:%*d:%*d:%*d:%*lld:%lld:%lld\n", &time_1, &time_2, &size_comm);

			burst_times[num_times].time = time_1;
			burst_times[num_times + 1].time = time_2;
			if(time_1 != time_2)
			{
				burst_times[num_times].value = size_comm/(time_2 - time_1);
				burst_times[num_times + 1].value = -(size_comm/(time_2 - time_1));
			}
			else
			{
				burst_times[num_times].value = 0;
				burst_times[num_times + 1].value = 0;
			}
			num_times += 2;

		}
	}

	if(option==5 || option==4)
	{
		/* Generating signalRunning */
		
		qsort(burst_times, num_times, sizeof(struct burst_info), qsort_cmp);
		
		if(burst_times[1].time - burst_times[0].time > 0)
		{
			signal[0].t=burst_times[0].time;
			signal[0].delta=burst_times[1].time - burst_times[0].time;
			signal[0].semanticvalue= burst_times[0].value;
			*size=1;
		}

		for(i = 1; i < num_times - 2; i++)
		{
			burst_times[i].value = burst_times[i].value + burst_times[i-1].value;
			if(burst_times[i+1].time - burst_times[i].time > 0)
			{
				signal[*size].t=burst_times[i].time;
				signal[*size].delta=burst_times[i+1].time - burst_times[i].time;
				signal[*size].semanticvalue=burst_times[i].value;
				*size=*size+1;	
			}
		}
	}
	else if(option == 2)
	{
		/* Generating signal IPC */
		double acumm_ipc = 0;
		*size=*size-1;
		for(i = 0; i < *size; i++)
		{
			signal[i].t=ipc_signal[i].time;
			signal[i].delta=ipc_signal[i+1].time - ipc_signal[i].time;
			signal[i].semanticvalue=acumm_ipc;
			acumm_ipc += ipc_signal[i].value;
		}
	}
	else if(option == 1)
	{
		/* Generating signalBW */
		
		qsort(burst_times, num_times, sizeof(struct burst_info), qsort_cmp);
		
		if(burst_times[0].time > 0)
		{
			signal[0].t=0;
			signal[0].delta=burst_times[0].time;
			signal[0].semanticvalue=0;
			*size=1;
		}

		for(i = 0; i < num_times - 2; i++)
		{
			if(burst_times[i+1].time - burst_times[i].time > 0)
			{
				signal[*size].t=burst_times[i].time;
				signal[*size].delta=burst_times[i+1].time - burst_times[i].time;
				signal[*size].semanticvalue=burst_times[i].value;
				*size=*size+1;
			}
		}
	}


	/* Generating signalDurRunning */
	qsort(burst_times2, num_times, sizeof(struct burst_info), qsort_cmp);

	if(burst_times2[1].time - burst_times2[0].time > 0)
	{
		signal3[0].t=burst_times2[0].time;
		signal3[0].delta=burst_times2[1].time - burst_times2[0].time;
		signal3[0].semanticvalue= (1.0 * burst_times2[0].value)/1000000.0;
		*size3=1;
	}

	for(i = 1; i < num_times - 2; i++)
	{
		burst_times2[i].value = burst_times2[i].value + burst_times2[i-1].value;
		if(burst_times2[i+1].time - burst_times2[i].time > 0)
		{
			signal3[*size3].t=burst_times2[i].time;
			signal3[*size3].delta=burst_times2[i+1].time - burst_times2[i].time;
			signal3[*size3].semanticvalue= (1.0 * burst_times2[i].value)/1000000.0;
			*size3=*size3+1;
		}
	}
	
	fclose(fileIn);
	free(buffer);
	free(burst_times2);
	
	if(option==5 || option==4 || option==1)
	{
		free(burst_times);
	}
	else if(option == 2)
	{
		free(ipc_signal);
		free(last_IPC);
	}
	//printf("%lld %lld %lf %d",signal[0].t,signal[0].delta,signal[0].semanticvalue,*size);
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}
#endif

int get_real_read_point(FILE *fileIn, int read_point)
{

	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	int size_done=0;
	char c=' ';
	
	//ir al read point
	fseek(fileIn, read_point, SEEK_SET);

	//buscar el real read_point y actualizar size_done
	while( (c != '\n') && ((c=(char)fgetc(fileIn)) != EOF) )
	{
		size_done++;
	}
	
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
	return size_done;
}
//#pragma omp task input(input,total_size,read_point) inout(burst_times,burst_times2,num_times)
void get_Burst_Running_DurRunning(char * input,int total_size,int read_point ,struct burst_info *burst_times,struct burst_info *burst_times2,unsigned long long *num_times)
{
	long long int time_1, time_2;
	int state,size_done;
	char *buffer;
	FILE *fileIn;
	/* open input file */
	if((fileIn = fopen(input, "r")) == NULL)
	{
		printf("get_FlushingSignal: Can't open file %s !!!\n\n", input);
		exit(-1);
	}
	
	/* Allocating memory for buffers */
	buffer = (char *)malloc(READ_BUFFER_SIZE);
	
	/*go to the real read point of the file (at start of a line) and know size_done*/
	
	size_done = get_real_read_point(fileIn,read_point);
	//*num_times=0;
	while( (size_done < total_size) && (fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL) )
	{
		//actualizar size actual hecho
		size_done += strlen(buffer);
		if(buffer[0] == '1')/* states */
		{
			sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%lld:%d\n", &time_1, &time_2, &state);
			if(state == 1)
			{
				//signalRunning
				burst_times[*num_times].time = time_1;
				burst_times[*num_times].value = 1;
				burst_times[*num_times+1].time = time_2;
				burst_times[*num_times+1].value = -1;
				
				//signalDurRunning
				burst_times2[*num_times].time = time_1;
				burst_times2[*num_times].value = time_2 - time_1;
				burst_times2[*num_times+1].time = time_2;
				burst_times2[*num_times+1].value = -(time_2 - time_1);
				*num_times=*num_times+2;
			}
		}
		
	}	
	fclose(fileIn);
}

void get_Burst_DurRunning(char * input,int total_size,int read_point ,struct burst_info *burst_times,unsigned long long *num_times)
{
	long long int time_1, time_2;
	int state,size_done;
	char *buffer;
	FILE *fileIn;
	/* open input file */
	if((fileIn = fopen(input, "r")) == NULL)
	{
		printf("get_FlushingSignal: Can't open file %s !!!\n\n", input);
		exit(-1);
	}
	
	/* Allocating memory for buffers */
	buffer = (char *)malloc(READ_BUFFER_SIZE);
	
	/*go to the real read point of the file (at start of a line) and know size_done*/
	
	size_done = get_real_read_point(fileIn,read_point);
	//*num_times=0;
	while( (size_done < total_size) && (fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL) )
	{
		//actualizar size actual hecho
		size_done += strlen(buffer);
		if(buffer[0] == '1')/* states */
		{
			sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%lld:%d\n", &time_1, &time_2, &state);
			if(state == 1)
			{				
				//signalDurRunning
				burst_times[*num_times].time = time_1;
				burst_times[*num_times].value = time_2 - time_1;
				burst_times[*num_times+1].time = time_2;
				burst_times[*num_times+1].value = -(time_2 - time_1);
				*num_times=*num_times+2;
			}
		}
		
	}	
	fclose(fileIn);
}

//#pragma omp task input(input,total_size,read_point) inout(signal2,size2,last_time,signalValue)
void get_FlushingSignal(char * input,int total_size,int read_point ,struct signal_info *signal2,int *size2,long long int *last_time,long long int *signalValue)
{
	long long int type, value;
	long long int current_time;
	int size_done;
	char events[4096], *token;
	char *buffer;
	FILE *fileIn;
	/* open input file */
	if((fileIn = fopen(input, "r")) == NULL)
	{
		printf("get_FlushingSignal: Can't open file %s !!!\n\n", input);
		exit(-1);
	}
	
	/* Allocating memory for buffers */
	buffer = (char *)malloc(READ_BUFFER_SIZE);
	
	/*go to the real read point of the file (at start of a line) and know size_done*/
	
	size_done = get_real_read_point(fileIn,read_point);
	
	while( (size_done < total_size) && (fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL) )
	{
		//actualizar size actual hecho
		size_done += strlen(buffer);
		
		if(buffer[0] == '2')/* events */
		{
			//GenerateEventSignal
			
			sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%s\n", &current_time, events);
			//sscanf(buffer, "%*d:%d:%*d:%*d:%*d:%lld:%s\n", &cpu, &current_time, events);
			token = strtok(events, ":");
			
			do
			{
				/* Obtaining type and value */
				type = atoll(token);
				value = atoll(strtok(NULL, ":"));

				/* Generating Signal */
				if(type == EVENT_FLUSHING)//EventSignal for flushing
				{
					if(current_time != *last_time)
					{
						signal2[*size2].t=*last_time;
						signal2[*size2].delta=current_time - *last_time;
						signal2[*size2].semanticvalue=*signalValue;
						*last_time = current_time;
						*size2 = *size2 + 1;
					}

					if(value != 0) *signalValue=*signalValue+1;
					else *signalValue=*signalValue-1;
					
					break;
				}
				
			}
			while((token = strtok(NULL, ":"))!=NULL);
			
		}
	}
	fclose(fileIn);
}

void get_MPIp2p_Flushing_Signal(char * input,int total_size,int read_point ,struct signal_info *signal,int *size,long long int *last_time,long long int *signalValue,struct signal_info *signal2,int *size2,long long int *last_time2,long long int *signalValue2)
{
	//signal-->MPIp2p
	//signal2-->flushing
	long long int type, value;
	long long int current_time;
	int size_done;
	char events[4096], *token;
	char *buffer;
	FILE *fileIn;
	/* open input file */
	if((fileIn = fopen(input, "r")) == NULL)
	{
		printf("get_FlushingSignal: Can't open file %s !!!\n\n", input);
		exit(-1);
	}
	
	/* Allocating memory for buffers */
	buffer = (char *)malloc(READ_BUFFER_SIZE);
	
	/*go to the real read point of the file (at start of a line) and know size_done*/
	
	size_done = get_real_read_point(fileIn,read_point);
	
	while( (size_done < total_size) && (fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL) )
	{
		//actualizar size actual hecho
		size_done += strlen(buffer);
		
		if(buffer[0] == '2')/* events */
		{
			//GenerateEventSignal
			
			sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%s\n", &current_time, events);
			//sscanf(buffer, "%*d:%d:%*d:%*d:%*d:%lld:%s\n", &cpu, &current_time, events);
			token = strtok(events, ":");
			
			do
			{
				/* Obtaining type and value */
				type = atoll(token);
				value = atoll(strtok(NULL, ":"));

				/* Generating Signal */
				if(type == EVENT_FLUSHING)//EventSignal for flushing
				{
					if(current_time != *last_time2)
					{
						signal2[*size2].t=*last_time2;
						signal2[*size2].delta=current_time - *last_time2;
						signal2[*size2].semanticvalue=*signalValue2;
						*last_time = current_time;
						*size2 = *size2 + 1;
					}

					if(value != 0) *signalValue2=*signalValue2+1;
					else *signalValue2=*signalValue2-1;
					
					//break;
				}
				else if(type == EVENT_MPIp2p)//EventSignal for MPIp2p
				{
					if(current_time != *last_time)
					{
						signal[*size].t=*last_time;
						signal[*size].delta=current_time - *last_time;
						signal[*size].semanticvalue=*signalValue;
						*last_time = current_time;
						
						*size = *size + 1;
						
					}
					if(value != 0) *signalValue=*signalValue+1;
					else *signalValue=*signalValue-1;
				}
				
			}
			while((token = strtok(NULL, ":"))!=NULL);
			
		}
	}
	fclose(fileIn);
}
//se podria buscar IPC a parte como un Burts, aqui el unico que es dependiente es flushing
void get_IPC_Flushing_Signal(char * input,int total_size,int read_point ,struct IPC_info *ipc_signal,int *size,double *last_IPC,struct signal_info *signal2,int *size2,long long int *last_time,long long int *signalValue)
{
	
	long long int instructions, cycles;
	long long int type, value;
	long long int current_time;
	int size_done;
	int counter_found, cpu;
	char events[4096], *token;
	char *buffer;
	FILE *fileIn;
	/* open input file */
	if((fileIn = fopen(input, "r")) == NULL)
	{
		printf("get_FlushingSignal: Can't open file %s !!!\n\n", input);
		exit(-1);
	}
	
	/* Allocating memory for buffers */
	buffer = (char *)malloc(READ_BUFFER_SIZE);
	
	/*go to the real read point of the file (at start of a line) and know size_done*/
	
	size_done = get_real_read_point(fileIn,read_point);
	
	while( (size_done < total_size) && (fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL) )
	{
		//actualizar size actual hecho
		size_done += strlen(buffer);
		
		if(buffer[0] == '2')/* events */
		{
			//GenerateEventSignal
			
			sscanf(buffer, "%*d:%d:%*d:%*d:%*d:%lld:%s\n", &cpu, &current_time, events);
			token = strtok(events, ":");
			
			do
			{
				/* Obtaining type and value */
				type = atoll(token);
				value = atoll(strtok(NULL, ":"));

				/* Generating Signal */
				if(type == EVENT_FLUSHING)//EventSignal for flushing
				{
					if(current_time != *last_time)
					{
						signal2[*size2].t=*last_time;
						signal2[*size2].delta=current_time - *last_time;
						signal2[*size2].semanticvalue=*signalValue;
						*last_time = current_time;
						*size2 = *size2 + 1;
					}

					if(value != 0) *signalValue=*signalValue+1;
					else *signalValue=*signalValue-1;
					
					//break;
				}else if(type == HW_COUNTER_INSTR)//EventSignal IPC
				{
					counter_found = 1;
					instructions = value;
				}
				else if(type == HW_COUNTER_CYC)//EventSignal IPC
				{
					counter_found = 1;
					cycles = value;
				}
				
			}
			while((token = strtok(NULL, ":"))!=NULL);
			
			if(counter_found)
			{
				ipc_signal[*size].time = current_time;
				if(cycles != 0)
					ipc_signal[*size].value = ((double)instructions/(double)cycles) - last_IPC[cpu];
				else
					ipc_signal[*size].value = 0.0;

				last_IPC[cpu] = ipc_signal[*size].value;
				*size=*size+1;
			}
		}
	}
	fclose(fileIn);
}
void get_Burst_BW_DurRunning(char * input,int total_size,int read_point ,struct burst_info *burst_times,struct burst_info *burst_times2,unsigned long long *num_times)
{
	long long int time_1, time_2,size_comm;
	int state,size_done;
	char *buffer;
	FILE *fileIn;
	/* open input file */
	if((fileIn = fopen(input, "r")) == NULL)
	{
		printf("get_FlushingSignal: Can't open file %s !!!\n\n", input);
		exit(-1);
	}
	
	/* Allocating memory for buffers */
	buffer = (char *)malloc(READ_BUFFER_SIZE);
	
	/*go to the real read point of the file (at start of a line) and know size_done*/
	
	size_done = get_real_read_point(fileIn,read_point);
	//*num_times=0;
	while( (size_done < total_size) && (fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL) )
	{
		//actualizar size actual hecho
		size_done += strlen(buffer);
		if(buffer[0] == '1')/* states */
		{
			sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%lld:%d\n", &time_1, &time_2, &state);
			if(state == 1)
			{
				
				//signalDurRunning
				burst_times2[*num_times].time = time_1;
				burst_times2[*num_times].value = time_2 - time_1;
				burst_times2[*num_times+1].time = time_2;
				burst_times2[*num_times+1].value = -(time_2 - time_1);
				*num_times=*num_times+2;
			}
		}
		else if(buffer[0] == '3') /* comm */
		{
			sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%*lld:%lld:%*d:%*d:%*d:%*d:%*lld:%lld:%lld\n", &time_1, &time_2, &size_comm);

			burst_times[*num_times].time = time_1;
			burst_times[*num_times + 1].time = time_2;
			if(time_1 != time_2)
			{
				burst_times[*num_times].value = size_comm/(time_2 - time_1);
				burst_times[*num_times + 1].value = -(size_comm/(time_2 - time_1));
			}
			else
			{
				burst_times[*num_times].value = 0;
				burst_times[*num_times + 1].value = 0;
			}
			*num_times += 2;

		}
		
	}	
	fclose(fileIn);
}
	
	

void FilterRunning(char * input, char * output, long long int t,int option)
{
	/**pre:
	 * 	-input: input trace to filter
	 * 	-output: output trace name for the result name
	 * 	-t: min time to show states
	 * 	-option:
	 * 		-1: incluir evento para MPIp2p EVENT_MPIp2p
	 * 		-2: incluir evento para IPC HW_COUNTER_CYC y HW_COUNTER_INSTR
	 * 		-3: incluir comunicaciones
	 * 		-otros: no incluir ningun evento extra solo EVENT_FLUSHING
	 * 
	 * post: @return output trace filtrada con t y segun la variable option
	 **/
	
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	
	char st[200],sb[200];
	
	sprintf(st,"-show_states_min_time:%lld",t);
	
	if(option ==1) sprintf(sb,"-show_events:%d,%d",EVENT_FLUSHING,EVENT_MPIp2p);
	else if(option ==2) sprintf(sb,"-show_events:%d,%d,%d",EVENT_FLUSHING,HW_COUNTER_CYC,HW_COUNTER_INSTR);
	else sprintf(sb,"-show_events:%d",EVENT_FLUSHING);
	
		
	if(option == 3)
	{
		char sa[12];
		sprintf(sa,"-show_comms");
		char *aux[6]={"trace_filter",input,output,sb,st,sa};
		trace_filter_main(6,aux);
	}
	else  
	{
		char *aux[5]={"trace_filter",input,output,sb,st};
		trace_filter_main(5,aux);
	}
	
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}

void GetTime(struct signalFreq_info *conj,int N, long long int *T2,int *correctOut, double *goodness, double *goodness2, double *goodness3, int *zigazagaOut, int *nzeros)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
		
	double p, f, p1, p2, f1, f2, maxp, maxf, maxp2, maxf2, minp, pant, fant, *max_, *max2;
	long long int t, i, j;
	int correct, zigazaga;

	maxp=0;
	maxp2=0;
	minp=10000000000;
	correct=1;
	max_ = (double *)malloc(sizeof(double) * N);

	//i=0;
	//---> Freqx2.txt ---> fprintf(gp, "%lf %lf\n", 1.0*i*freq, conj[i]);
	
	f2=0;
	p2=conj[0].conj;
	
	i=1;
	while((conj[i].conj==p2) && (i<N/2+3))
	{
		i++;
	}
	
	fant=conj[i].freq;
	pant=conj[i].conj;
	
	p1=pant;
	f1=fant;

	/* Provar d'inicialitzar les variables?*/

	max2=max_;

	for(j=i+1; j<N/2+3; j++)
	{
		f=conj[j].freq;
		p=conj[j].conj;

		if(p1!=p)
		{
			if(p2<p1 && p1>p)
			{
				*max_=f1;
				max_++;
				if(p1>maxp)
				{
					maxp=p1;
					maxf=f1;
				}
				else if(p1>maxp2 && p1<maxp)
				{
					maxp2=p1;
					maxf2=f1;
				}
				if(p1<minp)
				{
					minp=p1;
				}
			}

			p2=p1;
			f2=f1;
			p1=p;
			f1=f;
		}

	}
	
	*max_=-1.0;
	max_=max2;

	//printf("Period=%lf ms\n", maxf/1000000);
		
	if((maxp2/maxp < 0.9) || (MIN(maxf2, maxf) / MAX(maxf2, maxf) < 0.9) || (minp/maxp > 0.99))
	{
		//printf("Warning!!!! -----> Two similar periods\n");
		correct=0;
	}

	j=2;
	zigazaga=1;
	t=maxf;	
	i=0;

	#ifdef DEBUG_MODE
		FILE *fDB1;
		fDB1 = fopen("output.txt", "a+");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file output.txt !!!\n\n");
			exit(-1);
		}
	#endif
		
	while( (i<N) && (j<5) && (zigazaga==1))
	{
		f=conj[i].freq;
		p=conj[i].conj;
		#ifdef DEBUG_MODE
			fprintf(fDB1, "%lf %lf\n", 1.0*j*t, f);
		#endif
		if(/*0.95*f < j*t & j*t < 1.05*f*/ f==j*t) 
		{
			#ifdef DEBUG_MODE
				fprintf(fDB1, "%lf\n", *max_);
			#endif
			while((*max_ < f) && (*max_!=-1))
			{
				max_++;
			}

			if((0.95*f < *max_) && (*max_ < 1.05*f))
			{
				#ifdef DEBUG_MODE
					fprintf(fDB1, "%lf %lf %lf %lf\n", maxp, maxf, p, f);
				#endif
					
				j++;
			}
			else
			{
				zigazaga=0;
			}
		}
		i++;
	}

	if(j<5)
	{
		zigazaga=0;
	}
	

	if( (maxf2/maxf<=2.05 && maxf2/maxf>=1.95) || (maxf2/maxf<=3.05 && maxf2/maxf>=2.95)) 
	{
		zigazaga=1;
	}

	//Tcorrect 
	*T2 = t;
	*correctOut = correct;
	*goodness = maxp2/maxp;
	*goodness2 = maxf2/maxf;
	*goodness3 = minp/maxp;
	*zigazagaOut = zigazaga;
	*nzeros = j;
	
	#ifdef DEBUG_MODE
		fclose(fDB1);
		
		fDB1 = fopen("Tcorrect", "w");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file Tcorrect !!!\n\n");
			exit(-1);
		}
		fprintf(fDB1, "%lld %d %lf %lf %lf %d %lld\n", t, correct, maxp2/maxp, maxf2/maxf, minp/maxp, zigazaga, j);
		fclose(fDB1);
	#endif
	
	//free(max_);
	
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif

}
