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
#include "SenyalD.h"
#include "SenyalE.h"
#include "wavelet.h"
#include "fftprev.h"
#include "fft.h"

void ini_rand()
{
	long int t;

	t = time(NULL);
	srandom((unsigned int) t);
}


void getValues(int size,struct signalFloat_info *signal, long long int *t)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	int i = 0;
	int j=0;

	while(j<size)
	{
		if (signal[j].semanticvalue!=0.0)
		{
			t[i]=signal[j].t;
			i++;
			t[i]=signal[j].t+signal[j].delta;
			j++;
			while((j<size) && (signal[j].semanticvalue!=0.0))
			{
				t[i]+=signal[j].delta;
				j++;
			}
			i++;
		}
		j++;
	}
	
	t[i]=-1;
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}

#pragma omp task input(trace[*sizeIN],totaltime,sizeIN[1],totaltimeAux6) output(i[1],t0_flushing[200],t1_flushing[200])
void GetBoundary(struct signal_info *trace, long long int totaltime,int *sizeIN,long long int totaltimeAux6,int *i,long int *t0_flushing,long int *t1_flushing)
{
	
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	int sizeE,sizeD;
	struct signalFloat_info *signal;
	struct signalFloat_info *signal2;
	int size=*sizeIN;
	long long int tBoundary[1000];
	long long int a, b;
	#ifdef DEBUG_MODE
		FILE *fp;
	#endif
	if(size>0)
	{
		#ifdef DEBUG_MODE
			if((fp = fopen("KK", "w")) == NULL)
			{
				printf("\nDebug: Can't open file KK !!!\n\n");
				exit(-1);
			}
			fprintf(fp, "%lld", (long long int)(totaltime * 0.001));
			fclose(fp);
		#endif
		signal = (struct signalFloat_info *)malloc( (6*size+3) * sizeof(struct signalFloat_info));
		sizeD=SenyalD_exec(trace, totaltime * 0.001,size,signal);
		
		signal2 = (struct signalFloat_info *)malloc( (6*sizeD+3) * sizeof(struct signalFloat_info));
		sizeE=SenyalE_exec(totaltime * 0.001,sizeD,signal,signal2);
		getValues(sizeE,signal2, tBoundary);
		
		free(signal);
		free(signal2);		
	}
	else
	{
		tBoundary[0]=-1;
	}
	#ifdef DEBUG_MODE
		if((fp = fopen("tall.txt", "w")) == NULL)
		{
			printf("\nDebug: Can't open file tall.txt !!!\n\n");
			exit(-1);
		}
		int k=0;
		while(tBoundary[k]!=-1)
		{
			fprintf(fp, "%lld %lld\n", tBoundary[k], tBoundary[k+1]);
			k=k+2;
		}
		if(tBoundary[0]==-1) fprintf(fp, "0\n");
		fclose(fp);
	#endif
	free(trace);
	
	
	
	/* we get the boundary of the flushing regions */
	#ifdef TRACE_MODE
			Extrae_event (1000, 1);
	#endif
			
	*i=0;
	int j=0;
	while(tBoundary[j]!=-1)
	{
		a=tBoundary[j]/1000000;
		b=tBoundary[j+1]/1000000;
		if(a>0 && a<totaltimeAux6)
		{
			t1_flushing[*i]=a;
			if(b<totaltimeAux6)
			{
				*i=*i+1;
				t0_flushing[*i]=b;
				t1_flushing[*i]=totaltimeAux6;
			}
		}
		else if(a<=0 && b<totaltimeAux6)
		{
			t0_flushing[*i]=b;
			t1_flushing[*i]=totaltimeAux6;
		}
		else if (a>totaltimeAux6)
		{
			t1_flushing[*i]=totaltimeAux6;
		}
		
		j+=2;

	}
	#ifdef TRACE_MODE
		Extrae_event (1000, 0);
		Extrae_user_function(0);
	#endif
}


void GetPeriod(struct signalFloat_info *signal,int sizeSig, long long int Freqx,long long int deltaIN,long long int *T2,int *correct, double *goodness, double *goodness2, double *goodness3, int *zigazaga, int *nzeros)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	int M;
	fftw_complex *in, *out;
	struct signalFreq_info *conj;
	
	#ifdef TRACE_MODE
		Extrae_event (1000, 8);
	#endif
			
	#ifdef DEBUG_MODE
		FILE *fDB1;
		fDB1 = fopen("signalx2.txt", "w");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file signalx2.txt !!!\n\n");
			exit(-1);
		}
		int k=0;
		while (k<sizeSig)
		{
			fprintf(fDB1, "%lld %lld %f\n", signal[k].t, signal[k].delta, signal[k].semanticvalue);
			k++;
		}
	#endif
	//Correction
	if((signal[sizeSig-1].t+signal[sizeSig-1].delta)/1000000 < deltaIN)
	{
		signal[sizeSig].t=signal[sizeSig-1].t+signal[sizeSig-1].delta;
		signal[sizeSig].delta=deltaIN*1000000-signal[sizeSig-1].t-signal[sizeSig-1].delta;
		signal[sizeSig].semanticvalue=0.0;
		
		#ifdef DEBUG_MODE
			fprintf(fDB1, "%lld %lld 0\n",  signal[sizeSig].t, signal[sizeSig].delta);
		#endif
		
		sizeSig++;
	}
	
	#ifdef DEBUG_MODE
		fclose(fDB1);
	#endif
		
	#ifdef TRACE_MODE
		Extrae_event (1000, 0);
	#endif
	
	M=(signal[sizeSig-1].t+signal[sizeSig-1].delta)/Freqx+1;
	
	in=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*M);
	bzero(in, sizeof(fftw_complex)*M);
	
	out=(fftw_complex*)fftw_malloc(sizeof(fftw_complex)*M);
	bzero(out, sizeof(fftw_complex)*M);
	
	Sampler_fftw(signal, sizeSig,in,M, Freqx);
	
	fftprev_exec(in,M,out);
	fftw_free(in); 
	
	conj=(struct signalFreq_info *)malloc(sizeof(struct signalFreq_info)*M);
	//bzero(conj, sizeof(double)*M);
		
	fft_exec(out,M,conj, Freqx);
	fftw_free(out);
	
	GetTime(conj,M,T2,correct,goodness,goodness2,goodness3,zigazaga, nzeros);
	
	free(conj);
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}

#pragma omp task input(span,T2,sizeSin) output(sinus[sizeSin])
void Generatesinus(long long int span, long long int T2,double *sinus,int sizeSin)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	//sinus_periods=4
	//N = sinus_periods*T2/acc+span/acc-sinus_periods*T2/acc+2 --> sizeSin
	long long int i,j=0;
	#ifdef DEBUG_MODE
		FILE *fDB1;
		fDB1 = fopen("sin.txt", "w");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file sin.txt !!!\n\n");
			exit(-1);
		}
	#endif
	
	for(i=0; i*5000000<4*T2; i++)
	{
		sinus[j]=sin(2*(3.1416)/T2*i*5000000)+1;
		
		#ifdef DEBUG_MODE
			fprintf(fDB1, "%lf\n",sinus[j]);
		#endif
			
		j++;
	}

	for(i=4*T2/5000000; i*5000000<span; i++)
	{
		sinus[j]=0;
		j++;
		#ifdef DEBUG_MODE
			fprintf(fDB1, "0\n");
		#endif
	}
	
	#ifdef DEBUG_MODE
		fclose(fDB1);
	#endif
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}


Period_t * Analysis(struct signal_info *signal,int sizeSig, long long int t0, long long int t1, long int duration, long int r0, long int r1,
char* option, file trace, int cut,struct signal_info *signal3,int sizeSig3,int* pfound,
long int* period, file signalout, long int* point1, long int* point2, file filename, FILE *out,
FILE *err, int num_chop, int requested_iters,
long long int sizeTraceOrig, long long int totaltimeTraceOrig)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	long long int min, min2, T2,Tcorr,Tprev, spectral_trace_size;
	long long int size,max, accuracy;
	double goodness, goodcorr, goodness2, goodcorr2, goodness3, goodcorr3, goodprev, goodprev2, goodprev3;
	int correct,j, zigazaga, nzeros, zigazagacorr, zigazagaprev, n_iters,sizeCutter,sizeSample,sizeSin;
	char *env_var;
	double *sinus,*signalSample;
	FILE *pFile, *pFile2;
	Period_t * currentPeriod = NULL;
	struct signalFloat_info *signalCutter;

	//fprintf(stderr, "flushing=%d, recursion=%d, pzone=%d\n", n, i, j2);

	
	//sizeSig+1 --> después en GetPeriod añade una posición más si es necesario (correction)
	signalCutter = (struct signalFloat_info *)malloc( (sizeSig+1) *sizeof(struct signalFloat_info));
	sizeCutter=Cutter_signal_delta(signal,sizeSig ,signalCutter, t0*1000000, t1*1000000);
	#ifdef DEBUG_MODE
		FILE *fDB1;
		fDB1 = fopen("outin.txt", "w");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file outin.txt !!!\n\n");
			exit(-1);
		}
		j=0;
		while (j<sizeCutter)
		{
			fprintf(fDB1, "%lld %lld %f\n", signalCutter[j].t, signalCutter[j].delta, signalCutter[j].semanticvalue);
			j++;
		}
		fclose(fDB1);
	#endif
	
	min=t0*1000000;
	correct=0;
	T2=1000000;
	Tcorr=-1;
	zigazaga=0;
	goodcorr=0;

	/* Looking for the main period in signalCutter signal. Condicio correct==0 afegida!!*/
	
	accuracy=500000;
	while((T2/1000000!=0 && zigazaga==0 && T2/1000000<(t1-t0)/3.5 && correct==0) || (T2/1000000!=0 &&
		T2/1000000<(t1-t0)/3.5 &&(1.0*(t1-t0))/(1.0*T2/1000000)>300))
	{
		//GetPeriod(signalCutter,sizeCutter, accuracy,t1-t0,Tcorrect, tmp_dirname);
		// Accuracy = 5000000 usualment !!!!
		
		Tprev=T2;
		goodprev=goodness;
		goodprev2=goodness2;
		goodprev3=goodness3;
		zigazagaprev=zigazaga;
		
		GetPeriod(signalCutter,sizeCutter, accuracy,t1-t0,&T2, &correct, &goodness, &goodness2, &goodness3, &zigazaga, &nzeros);
		
		#ifdef TRACE_MODE
			Extrae_event (1000, 5);
		#endif
		if(((correct==1 && goodcorr<goodness) || (zigazaga==1 && goodcorr < goodness))&&(1.0*(t1-t0))/(1.0*T2/1000000)>2.5)
		{

			Tcorr=T2;
			goodcorr=goodness;
			goodcorr2=goodness2;
			goodcorr3=goodness3;
			zigazagacorr=zigazaga;
		}
		fprintf(err, "T2=%lld ns\n", T2);
		fprintf(err, "Accuracy=%lld ns\n", accuracy);
		fprintf(err, "Iters=%f, correct=%d, goodness=%lf, goodness2=%lf, goodness3=%lf, zz=%d, nz=%d\n", (1.0*(t1-t0))/(1.0*T2/1000000), correct, goodness, goodness2, goodness3, zigazaga, nzeros);
		
		accuracy=accuracy+T2/10;
		#ifdef TRACE_MODE
			Extrae_event (1000, 0);
		#endif
	}
	
	//printf("T2=%lld zigazaga=%d (t1-t0)/2=%ld correct=%d\n", T2, zigazaga, (t1-t0)/2, correct);
	if(Tcorr!=-1)
	{
		T2=Tcorr;
		goodness=goodcorr;
		goodness2=goodcorr2;
		goodness3=goodcorr3;
		zigazaga=zigazagacorr;
	}
	else
	{
		T2=Tprev;
		goodness=goodprev;
		goodness2=goodprev2;
		goodness3=goodprev3;
		zigazaga=zigazagaprev;
	}
	fprintf(err, "Main Period=%lld Iters=%f\n", T2/1000000, (1.0*(t1-t0))/(1.0*T2/1000000));
	
	if (T2>999999 && ((goodness > 0.7 && goodness < 1.3 && goodness2 > 0.7 && goodness2< 10.3 &&
		goodness3 < 0.9) || zigazaga==1) && cut==1 && T2/1000000<(t1-t0)/3.5)
	{
		fprintf(out, "Region from %ld to %ld ms\nStats:\nStructure:\n   Iters=%f Main Period=%lld ms Likeliness=(%lf,%lf,%lf)\n"
			     , r0, r1, (1.0*(duration))/(1.0*T2/1000000), T2/1000000, goodness, goodness2, goodness3);
		fflush(out);

		/* Save this period into the result vector */
		currentPeriod = (Period_t *)malloc(sizeof(Period_t));
		currentPeriod->iters = (1.0*(duration))/(1.0*T2/1000000);
		currentPeriod->length = T2/1000000;
		currentPeriod->goodness = goodness;
		currentPeriod->goodness2= goodness2;
		currentPeriod->goodness3= goodness3;
		currentPeriod->ini = t0 * 1000000;
		currentPeriod->end = t1 * 1000000;

		*pfound=1;
		*period=T2/1000000;

	}
	else
	{
		/*for(count=0;count<i+1; count++) {printf("   ");}
		printf("No period found\n");*/
		T2=0;
	}

	if(trace!=NULL)
	{
		sprintf(filename, "%s.%s.chop%d.prv", trace, option, num_chop);

	}

	/*Identifying 2 periods */

	/* Crosscorr*/
	//sinus_periods=4
	//N = sinus_periods*T2/acc+span/acc-sinus_periods*T2/acc+2
	sizeSin=(4*T2/5000000+((t1-t0)*1000000)/5000000-4*T2/5000000+2);
	sinus=(double *)malloc(sizeof(double)*sizeSin);
	// M=(signal[N-1].t+signal[N-1].delta)/freq+1
	sizeSample=(signalCutter[sizeCutter-1].t+signalCutter[sizeCutter-1].delta)/5000000+1;
	signalSample=(double *)malloc(sizeof(double)*sizeSample);
	
	/**Start Parallel**/
	Generatesinus((t1-t0)*1000000, T2,sinus,sizeSin);//para paralelo necesita el sizeSin
	Sampler_double(signalCutter,sizeCutter, signalSample, sizeSample,(long long int)5000000);
	Crosscorrelation(signalSample,sizeSample,sinus,&min2);
	/**End Parallel**/
	#pragma omp taskwait
	free(signalCutter);
	free(sinus);
	free(signalSample);
	
	fprintf(err, "Min accordin to sinus=%lld\n", min+min2*5000000);

	/* We cut the original trace in order to provide a 2 periods trace. */

	if(signal3!=NULL)
	{
		max=Cutter_signal_Maximum(signal3,sizeSig3 , min+min2*5000000, min+min2*5000000+T2);
	}
	else
	{
		max=Cutter_signal_Maximum(signal,sizeSig,min+min2*5000000, min+min2*5000000+T2);
	}
		
	/* tall sensible al tamany */
	if (trace != NULL && T2!=0)
	{

		printf("\nCalculating size of 1 iteration..."); fflush(stdout);
		min2=min2/2;
		
		
		#ifdef TRACE_MODE
			Extrae_event (1000, 9);
		#endif
		/*
		//tanto porciento del period respecto al total time de la traza original
		double itersPeriod = (T2*100.0)/(totaltimeTraceOrig*1.0);
		//lo que ocupa una iteracion
		size =(itersPeriod*sizeTraceOrig)/100;
		*/
		
		//Heuristico uniforme
		//size a partir del tanto porciento de un periodo al size de la traza de manera uniforme
		//size =(T2*sizeTraceOrig)/totaltimeTraceOrig;
		
		//Heuristico caso peor
		size =sizeTraceOrig/currentPeriod->iters;
		
		//Heuristico promedio
		//size = ( ((T2*sizeTraceOrig)/totaltimeTraceOrig) + sizeTraceOrig/currentPeriod->iters )/2;
		
		#ifdef TRACE_MODE
			Extrae_event (1000, 0);
		#endif
			
		printf("Done!\n"); fflush(stdout);
		fprintf(err, "size=%lld\n", size);

		/* We look for the environment var in order to get max trace size */
		if((env_var = getenv("SPECTRAL_TRACE_SIZE")) != NULL)
			spectral_trace_size = atoll(env_var);
		else
			spectral_trace_size = 100000000;
		
		max=max+2*T2;
		
		//numero iteraciones que caben en el tamño deseado spectral_trace_size
		if (size*2 > spectral_trace_size)
		{
			fprintf(err, "1111\n");
			n_iters=2;
		}
		else
		{
			fprintf(err, "2222\n");
			n_iters= spectral_trace_size/size;
		}

		if (min+min2*5000000+max+n_iters*T2 > t1*1000000)
		{
			if (n_iters > 2)
			{

				while (n_iters * T2 > (t1-t0) * 1000000 - max)
				{
					n_iters--;
				}
				n_iters--;
				min2 =  (t1-t0)*1000000 - max - (n_iters*T2);
				min2 = min2 / 5000000;
				//FIXME controlar signal3!null como arriba
				max=Cutter_signal_Maximum(signal3,sizeSig3 , min+min2*5000000, min+min2*5000000+T2);
				
			}
			else
			{
				fprintf(err, "4444\n");
				max=max-2*T2;
			}

		}

	}
	else n_iters=MAX(requested_iters, 2);

	/* fi tall sensible al tamany */

	if (T2>999999 && ((goodness > 0.7 && goodness < 1.3 && goodness2 > 0.7 && goodness2< 10.3 &&
		goodness3 < 0.9) || zigazaga==1)  && cut==1 && T2/1000000<(t1-t0)/3.5)
	{
		
		fprintf(out, "   Detected %d iters between %lld and %lld ns\n",
			n_iters, min+min2*5000000+max, min+min2*5000000+max+n_iters*T2);

		currentPeriod->best_ini = min+min2*5000000+max;
		currentPeriod->best_end = min+min2*5000000+max+n_iters*T2;

		/* Make sure the recommended iters are inside the region of data */
		while (currentPeriod->best_end > currentPeriod->end)
		{
			currentPeriod->best_ini -= currentPeriod->length * 1000000;
			currentPeriod->best_end -= currentPeriod->length * 1000000;
		}
		if (currentPeriod->best_ini < currentPeriod->ini)
		{
			currentPeriod->best_ini = currentPeriod->ini;
			fprintf(err, "Detected less iterations than requested!\n");
		}

		
		*point1=min/1000000+min2*5+max/1000000;
		*point2=min/1000000+min2*5+max/1000000+n_iters*T2/1000000;
		/**Start Parallel**/
		if(trace!=NULL)
		{
			Cutter2(trace, filename,  min+min2*5000000+max,
				min+min2*5000000+max+n_iters*T2);
		}
		
		Cutter_signal_OutFile(signal, sizeSig,signalout,  min+min2*5000000+max,
			min+min2*5000000+max+n_iters*T2);
		/**End Parallel**/
		#pragma omp taskwait

	}
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
	return currentPeriod;
}

#pragma omp task input(signal[1],sizeSig[1]) output(signal3[1],sizeSig3[1])
void signalChange(struct signal_info *signal,int *sizeSig,struct signal_info **signal3,int *sizeSig3)
{
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	//signal3 == signal --> signalDurRunning
	free(*signal3);
	*signal3=signal;
	*sizeSig3=*sizeSig;
	#ifdef DEBUG_MODE
		FILE *fDB1;	
		fDB1 = fopen("signal3.txt", "w");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file signal.txt !!!\n\n");
			exit(-1);
		}
		int j=0;
		while (j<*sizeSig3)
		{
			fprintf(fDB1, "%lld %lld %lf\n", signal[j].t,signal[j].delta, signal[j].semanticvalue);
			j++;
		}
		fclose(fDB1);
	#endif
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}


#ifndef LIB_MODE
int main(int argc, char *argv[])
{
	#ifdef TRACE_MODE
		Extrae_init();
		Extrae_user_function(1);
		//Extrae_event (1000, 0);
	#endif
	long int temps = time(NULL);
	long int t0_flushing[200], t1_flushing[200], c, d, *periods, t0, t1;
	long long int a, b, *totaltime, min, a2, b2,totaltimeAux6;
	long long int filt, size;
	int *p, n_points, sizeSig,sizeSig2,sizeSig3,NUM_SIGNALS=1;
	int j, i,k, n, pfound, counter, trace_num, change, num_chop,m;
	char filename[1024], systema[1024], **signals, **traces, s[10000];
	FILE *hp, *jp, *kp;
	double *waveletSignal;
	struct signal_info *signal,*signal2,*signal3;
	#ifdef OMPSS_MODE
		NUM_SIGNALS=omp_get_num_threads();
	#endif
	struct burst_info *burst_times[NUM_SIGNALS+1],*burst_times2[NUM_SIGNALS+1];
	struct IPC_info *ipc_signal;
	unsigned long long num_times[NUM_SIGNALS+1],num_times2[NUM_SIGNALS+1];
	
	long long int signalValue,last_time,signalValue2,last_time2;
	int total_size,offset;
	double *last_IPC;
	#ifdef DEBUG_MODE
		FILE *fDB1;
	#endif
		
	if (argc<3)
	{
		printf("Usage: %s <first prv file> <second prv file> ... <BW or MPIp2p or CPUBurst or CPUDurBurst or IPC>\n", argv[0]);
		exit (-1);
	}

	ini_rand();

	/* Ini structs */
	periods=(long int *)malloc(sizeof(long int) * (argc-2));
	totaltime=(long long int *)malloc(sizeof(long long int) * (argc-2));
	p=(int *)malloc(sizeof(int) * (argc-2));
	signals = (char **)malloc(sizeof(char*) * (argc-2));
	traces = (char **)malloc(sizeof(char*) * (argc-2));
	
	for(i=1; i<argc-1; i++)
	{
		signals[i]=(char *)malloc(sizeof(char) * 256);
		traces[i]=(char *)malloc(sizeof(char) * 256);
	}

	change=0;

	
	hp=fopen("report.out","w");

	jp=fopen("report.err","w");

	/*loop of the tracefiles*/
	
/* start SMPSs */
//#pragma css start
	for(trace_num=1; trace_num < argc-1; trace_num++)
	{
		kp=fopen(argv[trace_num],"r");
		/*Extraction of the execution time and the number of threads*/
		
		
		fgets(s, 10000, kp);
		#ifdef TRACE_MODE
			Extrae_event (1000, 7);
		#endif
		sscanf(s, "%*s %*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%d", &p[trace_num]);
		#ifdef TRACE_MODE
			Extrae_event (1000, 0);
			Extrae_event (1000, 6);
		#endif
		sscanf(s, "%*s %*[^:]:%*[^:]:%lld", &totaltime[trace_num]);
		#ifdef TRACE_MODE
			Extrae_event (1000, 0);
		#endif
			
			
		sprintf(signals[trace_num], "%s.1it.txt", argv[trace_num]);

		fprintf(hp, "Trace: %s\n", argv[trace_num]);
		fprintf(hp, "Metric: %s\n", argv[argc-1]);
		fprintf(hp, "Totaltime=%lld ms\n", totaltime[trace_num]/1000000);
		fprintf(hp, "Totalthreads=%d\n", p[trace_num]);

		
		fseek(kp, 0L, SEEK_END);
		size=ftello64(kp);
		fclose(kp);
		printf("Size=%lld\n", size);
		fprintf(hp, "Size=%lld\n\n", size);

		/*we generate the signal we will study*/
		
		#ifdef TRACE_MODE
			Extrae_event (1000, 2);
		#endif
		sprintf(filename, "%s.%s.filtered.prv", argv[trace_num], argv[argc-1]);
		
		
		fprintf(hp, "Filtered trace: %s.%s.filtered.prv\n", argv[trace_num], argv[argc-1]);
		
		//init flushing
		totaltimeAux6=totaltime[trace_num]/1000000;
		t0_flushing[0]=0;
		t1_flushing[0]=totaltimeAux6;
		
		counter=0;
		pfound=0;
		num_chop = 0;
		
		
		/* get signal(signal to study), signal3(signal to get maximum in Analysis) and wavelet*/
		if(strcmp(argv[argc-1], "BW")==0)
		{
			FilterRunning(argv[trace_num], filename,  totaltime[trace_num]/10000000,3);
			n_points=1024;
			
			/*we generate the sematics signals*/
			/*
			signal = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//signalBW
			signal2 = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//GenerateEventSignal
			signal3 = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//SignalDurRunning
			burst_times = (struct burst_info *)malloc( SIZE_SIGNAL* sizeof(struct burst_info));
			burst_times2 = (struct burst_info *)malloc( SIZE_SIGNAL* sizeof(struct burst_info));
			waveletSignal = (double *)malloc(sizeof(double)*n_points);
			
			Generate_Event_BW_DurRunning(filename,signal2,&sizeSig2,burst_times,&num_times,burst_times2,&num_times2);
			
			signalBW_out(burst_times,&num_times,signal,&sizeSig);
			signalDurRunning_out(burst_times2,&num_times2,signal3,&sizeSig3);
			
			Sampler_wavelet(signal,&sizeSig,waveletSignal, n_points);
			*/
			for(j=0;j<NUM_SIGNALS+1;j++)
			{
				burst_times[j] = (struct burst_info *)malloc( SIZE_SIGNAL* sizeof(struct burst_info));
				burst_times2[j] = (struct burst_info *)malloc(SIZE_SIGNAL * sizeof(struct burst_info));
				num_times[j]=0;
			}
			
			waveletSignal = (double *)malloc(sizeof(double)*n_points);
			
			//Generate_Event_Running_DurRunning(filename,signal2,&sizeSig2,burst_times,burst_times2,&num_times);
			sizeSig2=0;
			
			/* open input file */
			if((kp = fopen(filename, "r")) == NULL)
			{
				printf("Generate_Event_Running_DurRunning: Can't open file %s !!!\n\n", filename);
				exit(-1);
			}
			
			//vars GenerateEventSignal 40000003
			last_time = 0; signalValue = 0;
			
			/* Parsing trace */
			
			//get size file
			fseek(kp, 0L, SEEK_END);
			total_size=ftello64(kp);
			fclose(kp);
			
			offset=total_size/NUM_SIGNALS;
			k=1;
			
			//Done falso, sino necesario un barrier
			printf("Generating Sematics Signals...Done!\n"); fflush(stdout);
/** Start real SMPSs functions **/
			
			//el primero no necesita merge_burst
			get_Burst_Running_DurRunning(filename,offset,0,burst_times[0],burst_times2[0],&num_times[0]);
			get_FlushingSignal(filename,offset,j,signal2,&sizeSig2,&last_time,&signalValue);
			//resto
			for(j=offset;j<total_size-offset+1;j+=offset)
			{
				#ifdef TRACE_MODE
					Extrae_event (1000, 11);
				#endif
				
				get_Burst_Running_DurRunning(filename,offset,j,burst_times[k],burst_times2[k],&num_times[k]);
				merge_burst(burst_times[0],burst_times2[0],&num_times[0],burst_times[k],burst_times2[k],&num_times[k]);
				get_FlushingSignal(filename,offset,j,signal2,&sizeSig2,&last_time,&signalValue);
				k++;
				
				#ifdef TRACE_MODE
					Extrae_event (1000, 0);
				#endif
			}
			
			//lo q queda
			if(offset*NUM_SIGNALS != total_size)
			{
				get_Burst_Running_DurRunning(filename,offset,offset*4,burst_times[k],burst_times2[k],&num_times[k]);
				merge_burst(burst_times[0],burst_times2[0],&num_times[0],burst_times[k],burst_times2[k],&num_times[k]);
				get_FlushingSignal(filename,offset,offset*4,signal2,&sizeSig2,&last_time,&signalValue);
				
			}
			
			signalBW_out(burst_times[0],&num_times[0],signal,&sizeSig);	
			signalDurRunning_out(burst_times2[0],&num_times[0],signal3,&sizeSig3);
			
			
			Sampler_wavelet(signal3,&sizeSig3,waveletSignal, n_points);
		}
		else if(strcmp(argv[argc-1], "IPC")==0)
		{
			FilterRunning(argv[trace_num], filename,  totaltime[trace_num]/10000000,2);
			n_points=4096;
			
			/*we generate the sematics signals*/
			signal = (struct signal_info *)malloc( SIZE_SIGNAL_IPC * sizeof(struct signal_info));//Signal IPC
			signal2 = (struct signal_info *)malloc( SIZE_SIGNAL_IPC * sizeof(struct signal_info));//GenerateEventSignal 40000003
			signal3 = (struct signal_info *)malloc( SIZE_SIGNAL_IPC * sizeof(struct signal_info));//SignalDurRunning
			/*burst_times = (struct burst_info *)malloc( SIZE_SIGNAL* sizeof(struct burst_info));
			ipc_signal = (struct IPC_info *)malloc(SIZE_SIGNAL_IPC * sizeof(struct IPC_info));
			waveletSignal = (double *)malloc(sizeof(double)*n_points);
			
			
			Generate_IPC_Event_DurRunning(filename,ipc_signal,&sizeSig,signal2,&sizeSig2,burst_times,&num_times);
			
			signalIPC_out(ipc_signal,&sizeSig,signal);
			signalDurRunning_out(burst_times,&num_times,signal3,&sizeSig3);
			
			Sampler_wavelet(signal,&sizeSig,waveletSignal, n_points);;
			*/
			last_IPC = (double *)malloc(1000 * sizeof(double));
			ipc_signal = (struct IPC_info *)malloc(SIZE_SIGNAL_IPC * sizeof(struct IPC_info));
			for(j=0;j<NUM_SIGNALS+1;j++)
			{
				burst_times[j] = (struct burst_info *)malloc( SIZE_SIGNAL* sizeof(struct burst_info));
				num_times[j]=0;
			}
			
			waveletSignal = (double *)malloc(sizeof(double)*n_points);
			
			sizeSig2=0;
			sizeSig=0;
			
			/* open input file */
			if((kp = fopen(filename, "r")) == NULL)
			{
				printf("Generate_Event_Running_DurRunning: Can't open file %s !!!\n\n", filename);
				exit(-1);
			}
			
			//vars GenerateEventSignal 40000003
			last_time = 0; signalValue = 0;
			
			//get size file
			fseek(kp, 0L, SEEK_END);
			total_size=ftello64(kp);
			fclose(kp);
			
			offset=total_size/NUM_SIGNALS;
			k=1;
			
			//Done falso, sino necesario un barrier
			printf("Generating Sematics Signals...Done!\n"); fflush(stdout);
/** Start real SMPSs functions **/
			
			//el primero no necesita merge_burst
			get_Burst_DurRunning(filename,offset,0,burst_times[0],&num_times[0]);
			get_IPC_Flushing_Signal(filename,offset,j,ipc_signal,&sizeSig,last_IPC,signal2,&sizeSig2,&last_time2,&signalValue2);
			//resto
			for(j=offset;j<total_size-offset+1;j+=offset)
			{	
				get_Burst_DurRunning(filename,offset,j,burst_times[k],&num_times[k]);
				merge_burst_one(burst_times[0],&num_times[0],burst_times[k],&num_times[k]);
				get_IPC_Flushing_Signal(filename,offset,j,ipc_signal,&sizeSig,last_IPC,signal2,&sizeSig2,&last_time2,&signalValue2);
				k++;
			}
			
			//lo que queda
			if(offset*NUM_SIGNALS != total_size)
			{
				get_Burst_DurRunning(filename,offset,offset*4,burst_times[k],&num_times[k]);
				merge_burst_one(burst_times[0],&num_times[0],burst_times[k],&num_times[k]);
				get_IPC_Flushing_Signal(filename,offset,offset*4,ipc_signal,&sizeSig,last_IPC,signal2,&sizeSig2,&last_time2,&signalValue2);
				
			}
			
			signalDurRunning_out(burst_times[0],&num_times[0],signal3,&sizeSig3);
			signalIPC_out(ipc_signal,&sizeSig,signal);
			Sampler_wavelet(signal,&sizeSig,waveletSignal, n_points);
		}
		else if(strcmp(argv[argc-1],"MPIp2p")==0)
		{
			FilterRunning(argv[trace_num], filename,  totaltime[trace_num]/10000000,1);
			n_points=4096;
			
			/*we generate the sematics signals*/
			signal = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//GenerateEventSignal 50000001
			signal2 = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//GenerateEventSignal 40000003
			signal3 = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//SignalDurRunning
			
			for(j=0;j<NUM_SIGNALS+1;j++)
			{
				burst_times[j] = (struct burst_info *)malloc( SIZE_SIGNAL* sizeof(struct burst_info));
				num_times[j]=0;
			}
			
			waveletSignal = (double *)malloc(sizeof(double)*n_points);
			
			//Generate_Event_Running_DurRunning(filename,signal2,&sizeSig2,burst_times,burst_times2,&num_times);
			sizeSig2=0;
			sizeSig=0;
			
			/* open input file */
			if((kp = fopen(filename, "r")) == NULL)
			{
				printf("Generate_Event_Running_DurRunning: Can't open file %s !!!\n\n", filename);
				exit(-1);
			}
			
			//vars GenerateEventSignal 40000003
			last_time = 0; signalValue = 0;
			
			//vars GenerateEventSignal 50000001
			last_time2 = 0; signalValue2 = 0;
			/* Parsing trace */
			
			//get size file
			fseek(kp, 0L, SEEK_END);
			total_size=ftello64(kp);
			fclose(kp);
			
			offset=total_size/NUM_SIGNALS;
			k=1;
			
			//Done falso, sino necesario un barrier
			printf("Generating Sematics Signals...Done!\n"); fflush(stdout);
/** Start real SMPSs functions **/
			
			//el primero no necesita merge_burst
			get_Burst_DurRunning(filename,offset,0,burst_times[0],&num_times[0]);
			get_MPIp2p_Flushing_Signal(filename,offset,j,signal,&sizeSig,&last_time,&signalValue,signal2,&sizeSig2,&last_time2,&signalValue2);
			//resto
			for(j=offset;j<total_size-offset+1;j+=offset)
			{	
				get_Burst_DurRunning(filename,offset,j,burst_times[k],&num_times[k]);
				merge_burst_one(burst_times[0],&num_times[0],burst_times[k],&num_times[k]);
				get_MPIp2p_Flushing_Signal(filename,offset,j,signal,&sizeSig,&last_time,&signalValue,signal2,&sizeSig2,&last_time2,&signalValue2);
				k++;
			}
			
			//lo que queda
			if(offset*NUM_SIGNALS != total_size)
			{
				get_Burst_DurRunning(filename,offset,offset*4,burst_times[k],&num_times[k]);
				merge_burst_one(burst_times[0],&num_times[0],burst_times[k],&num_times[k]);
				get_MPIp2p_Flushing_Signal(filename,offset,offset*4,signal,&sizeSig,&last_time,&signalValue,signal2,&sizeSig2,&last_time2,&signalValue2);
				
			}
			
			signalDurRunning_out(burst_times[0],&num_times[0],signal3,&sizeSig3);
				
			Sampler_wavelet(signal,&sizeSig,waveletSignal, n_points);
		}
		else if(strcmp(argv[argc-1],"CPUBurst")==0)
		{
			FilterRunning(argv[trace_num], filename,  totaltime[trace_num]/10000000,0);
			n_points=4096;
			
			/*we generate the sematics signals*/
			signal = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//SignalRunning
			signal2 = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//GenerateEventSignal 40000003
			signal3 = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//SignalDurRunning
			
			
			for(j=0;j<NUM_SIGNALS+1;j++)
			{
				burst_times[j] = (struct burst_info *)malloc( SIZE_SIGNAL* sizeof(struct burst_info));
				burst_times2[j] = (struct burst_info *)malloc(SIZE_SIGNAL * sizeof(struct burst_info));
				num_times[j]=0;
			}
			
			waveletSignal = (double *)malloc(sizeof(double)*n_points);
			
			//Generate_Event_Running_DurRunning(filename,signal2,&sizeSig2,burst_times,burst_times2,&num_times);
			sizeSig2=0;
			sizeSig=0;
			
			/* open input file */
			if((kp = fopen(filename, "r")) == NULL)
			{
				printf("Generate_Event_Running_DurRunning: Can't open file %s !!!\n\n", filename);
				exit(-1);
			}
			
			//vars GenerateEventSignal 40000003
			last_time = 0; signalValue = 0;
			
			/* Parsing trace */
			
			//get size file
			fseek(kp, 0L, SEEK_END);
			total_size=ftello64(kp);
			fclose(kp);
			
			offset=total_size/NUM_SIGNALS;
			k=1;
			
			//Done falso, sino necesario un barrier
			printf("Generating Sematics Signals...Done!\n"); fflush(stdout);
/** Start real SMPSs functions **/
			
			//el primero no necesita merge_burst
			get_Burst_Running_DurRunning(filename,offset,0,burst_times[0],burst_times2[0],&num_times[0]);
			get_FlushingSignal(filename,offset,j,signal2,&sizeSig2,&last_time,&signalValue);
			//resto
			for(j=offset;j<total_size-offset+1;j+=offset)
			{	
				get_Burst_Running_DurRunning(filename,offset,j,burst_times[k],burst_times2[k],&num_times[k]);
				merge_burst(burst_times[0],burst_times2[0],&num_times[0],burst_times[k],burst_times2[k],&num_times[k]);
				get_FlushingSignal(filename,offset,j,signal2,&sizeSig2,&last_time,&signalValue);
				k++;
			}
			
			//lo que queda
			if(offset*NUM_SIGNALS != total_size)
			{
				get_Burst_Running_DurRunning(filename,offset,offset*4,burst_times[k],burst_times2[k],&num_times[k]);
				merge_burst(burst_times[0],burst_times2[0],&num_times[0],burst_times[k],burst_times2[k],&num_times[k]);
				get_FlushingSignal(filename,offset,offset*4,signal2,&sizeSig2,&last_time,&signalValue);
				
			}
			
			signalRunning_out(burst_times[0],&num_times[0],signal,&sizeSig);
			signalDurRunning_out(burst_times2[0],&num_times[0],signal3,&sizeSig3);
			
			
			Sampler_wavelet(signal,&sizeSig,waveletSignal, n_points);
		}
		else if(strcmp(argv[argc-1],"CPUDurBurst")==0)
		{
			if (size<10000000000 || strcmp(argv[argc-1],"CPUDurBurstlog")==0 ) {n_points=4096;}
			else {n_points=655360;}

			FilterRunning(argv[trace_num], filename,  totaltime[trace_num]/100000,0);

			fprintf(hp, "Filtering=%lld ns\n", totaltime[trace_num]/100000);
			fprintf(hp, "N_points=%d \n", n_points);
			
			/*we generate the sematics signals*/
			//TODO q no haya un segfault con size 20000000 * 2 * sizeof(struct signal_info)
			signal = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//SignalDurRunning
			signal2 = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//GenerateEventSignal 40000003
			signal3 = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//SignalRunning
			
			//num_times = (unsigned long long)malloc(sizeof(unsigned long long)*(NUM_SIGNALS+1));
			for(j=0;j<NUM_SIGNALS+1;j++)
			{
				burst_times[j] = (struct burst_info *)malloc( SIZE_SIGNAL* sizeof(struct burst_info));
				burst_times2[j] = (struct burst_info *)malloc(SIZE_SIGNAL * sizeof(struct burst_info));
				num_times[j]=0;
			}
			
			waveletSignal = (double *)malloc(sizeof(double)*n_points);
			
			//Generate_Event_Running_DurRunning(filename,signal2,&sizeSig2,burst_times,burst_times2,&num_times);
			sizeSig2=0;
			
			/* open input file */
			if((kp = fopen(filename, "r")) == NULL)
			{
				printf("Generate_Event_Running_DurRunning: Can't open file %s !!!\n\n", filename);
				exit(-1);
			}
			
			//vars GenerateEventSignal 40000003
			last_time = 0; signalValue = 0;
			
			/* Parsing trace */
			
			//get size file
			fseek(kp, 0L, SEEK_END);
			total_size=ftello64(kp);
			fclose(kp);
			
			offset=total_size/NUM_SIGNALS;
			k=1;
			
			//Done falso, sino necesario un barrier
			printf("Generating Sematics Signals...Done!\n"); fflush(stdout);
/** Start real SMPSs functions **/
			
			//el primero no necesita merge_burst
			get_Burst_Running_DurRunning(filename,offset,0,burst_times[0],burst_times2[0],&num_times[0]);
			get_FlushingSignal(filename,offset,j,signal2,&sizeSig2,&last_time,&signalValue);
			//resto
			for(j=offset;j<total_size-offset+1;j+=offset)
			{
				#ifdef TRACE_MODE
					Extrae_event (1000, 11);
				#endif
				
				get_Burst_Running_DurRunning(filename,offset,j,burst_times[k],burst_times2[k],&num_times[k]);
				merge_burst(burst_times[0],burst_times2[0],&num_times[0],burst_times[k],burst_times2[k],&num_times[k]);
				get_FlushingSignal(filename,offset,j,signal2,&sizeSig2,&last_time,&signalValue);
				k++;
				
				#ifdef TRACE_MODE
					Extrae_event (1000, 0);
				#endif
			}
			
			//lo q queda
			if(offset*NUM_SIGNALS != total_size)
			{
				get_Burst_Running_DurRunning(filename,offset,offset*4,burst_times[k],burst_times2[k],&num_times[k]);
				merge_burst(burst_times[0],burst_times2[0],&num_times[0],burst_times[k],burst_times2[k],&num_times[k]);
				get_FlushingSignal(filename,offset,offset*4,signal2,&sizeSig2,&last_time,&signalValue);
				
			}
			
			signalRunning_out(burst_times[0],&num_times[0],signal3,&sizeSig3);
			signalDurRunning_out(burst_times2[0],&num_times[0],signal,&sizeSig);
			
			
			Sampler_wavelet(signal3,&sizeSig3,waveletSignal, n_points);
			
			
			//free(signal3);
			//signal3 == signal --> signalDurRunning
			//signal3=signal;
			//sizeSig3=sizeSig;
			signalChange(signal,&sizeSig,&signal3,&sizeSig3);
		}
		else
		{
			printf("Usage: %s <first prv file> <second prv file> ... <BW or MPIp2p or CPUBurst or CPUDurBurst or IPC>\n", argv[0]);
			exit (-1);
		}
		
		/* we get the boundary of the flushing regions */
		GetBoundary(signal2,totaltime[trace_num],&sizeSig2,totaltimeAux6,&i,t0_flushing,t1_flushing);
		
		/*Global analysis using wavelets */
		Wavelet_exec(waveletSignal,n_points,&sizeSig2);
		
#pragma omp taskwait
		
		#ifdef TRACE_MODE
			Extrae_event (1000, 0);
			Extrae_event (1000, 10);
		#endif	
			
		/*selection of the peridical regions without flushing*/		
		while(counter < 2 && pfound==0)
		{
			fprintf(hp, "Global Periodic Zones\n");
			
			#ifdef TRACE_MODE
				Extrae_event (1000, 3);
			#endif
			m=0;
			n=0;
			pfound=0;
			
			while(m<sizeSig2)
			{
				if(waveletSignal[m]<waveletSignal[m+1])
				{
					#ifdef TRACE_MODE
						Extrae_event (1000, 4);
					#endif
					c=totaltimeAux6*waveletSignal[m];
					d=totaltimeAux6*waveletSignal[m+1];
					a=0;
					b=0;
					a2=0;
					b2=0;
					fprintf(jp, "From %ld to %ld ms\n", c, d);
					
					min=0;
					
					for(j=0; j<=i; j++)
					{
						//printf("Flush: %ld %ld\n", t0_flushing[j], t1_flushing[j]);
						if(c<t0_flushing[j] && d>t1_flushing[j])
						{
							if(min<t1_flushing[j]- t0_flushing[j])
							{
								a=t0_flushing[j];
								b=t1_flushing[j];
								a2=t0_flushing[j+3];
								b2=t1_flushing[j+3];
								min=b-a;
							}
						}
						else if(t0_flushing[j]<=c &&  d>t1_flushing[j])
						{
							if(min<t1_flushing[j]-c)
							{
								a=c;
								b=t1_flushing[j];
								min=b-a;
							}
						}
						else if(c<t0_flushing[j] && t0_flushing[j]<=d & t1_flushing[j]>=d)
						{
							if(min<d - t0_flushing[j])
							{
								a=t0_flushing[j];
								b=d;
								min=b-a;
							}
						}
						else if(t0_flushing[j]<=c && t1_flushing[j]>=d)
						{
							if(min<d-c)
							{
								a=c;
								b=d;
								min=b-a;
							}
						}
					}
					fprintf(jp, "%lld %lld\n", a, b);
					#ifdef TRACE_MODE
						Extrae_event (1000, 0);
					#endif
					if(b!=0 && (d-c)>0.05*totaltimeAux6)
					{
						num_chop++;
						fprintf(hp, "\n");
						printf("\nExecuting analysis..."); fflush(stdout);
						
						Analysis(signal,sizeSig, a, b, d-c, c, d,argv[argc-1],
							argv[trace_num], 1,
							signal3,sizeSig3,&pfound, &periods[trace_num], signals[trace_num],
							&t0, &t1, traces[trace_num], hp, jp, num_chop, 0,
							size, totaltime[trace_num]
							);
							
					}
					n++;
					if(change==1)
					{ 
						if(signal3 == signal)
						{
							//signal3 == signal, so need new @
							signal = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));
							
						}
						//FIXME variable filt sin inicializar, debia venir de esto, pero estaba comentado en:
						//filt=Filtering(argv[trace_num], "-bursts_histo");
						FilterRunning(argv[trace_num], filename, filt,-1);
						sizeSig=SignalDurRunning(filename,signal);
						//Cutter_signal("signal.txt", signals[trace_num], t0, t1);
						Cutter_signal_OutFile(signal,sizeSig, signals[trace_num], t0, t1);
						#pragma omp taskwait
						#ifdef DEBUG_MODE
							//FILE *fDB1;
							fDB1 = fopen("signal.txt", "w");
							if (fDB1==NULL)
							{
								printf("\nDebug: Can't open file signal.txt !!!\n\n");
								exit(-1);
							}
							j=0;
							while (j<sizeSig)
							{
								fprintf(fDB1, "%lld %lld %lf\n", signal[j].t, signal[j].delta, signal[j].semanticvalue);
								j++;
							}
							fclose(fDB1);
						#endif
						
					}
				}
				m+=2;
			}
			
			#ifdef TRACE_MODE
				Extrae_event (1000, 0);
			#endif
			
			//Problema que cal solucionar !!
			if(pfound==0 && strcmp(argv[argc-1], "CPUBurst")!=0)
			{
				strcpy(argv[argc-1], "CPUBurst");
				counter++;
				fprintf(hp, "Metric: %s\n", argv[argc-1]);
				change=1;
				
				/*all calculations here of CPUBurst*/
				
				free(signal);
				if(signal!=signal3) free(signal3);
				free(waveletSignal);
				
				FilterRunning(argv[trace_num], filename,  totaltime[trace_num]/10000000,0);
				n_points=4096;
				
				/*we generate the sematics signals*/
				signal = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//SignalRunning
				signal2 = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//GenerateEventSignal 40000003
				signal3 = (struct signal_info *)malloc( SIZE_SIGNAL * sizeof(struct signal_info));//SignalDurRunning
				/*FIXME to real calculations
				burst_times = (struct burst_info *)malloc( SIZE_SIGNAL* sizeof(struct burst_info));
				burst_times2 = (struct burst_info *)malloc(SIZE_SIGNAL * sizeof(struct burst_info));
				
				waveletSignal = (double *)malloc(sizeof(double)*n_points);
				
				Generate_Event_Running_DurRunning(filename,signal2,&sizeSig2,burst_times,burst_times2,&num_times);
				
				signalRunning_out(burst_times,&num_times,signal,&sizeSig);
				signalDurRunning_out(burst_times2,&num_times,signal3,&sizeSig3);
				*/
				Sampler_wavelet(signal,&sizeSig,waveletSignal, n_points);
				
				/* we get the boundary of the flushing regions */
				GetBoundary(signal2,totaltime[trace_num],&sizeSig2,totaltimeAux6,&i,t0_flushing,t1_flushing);
				
				/*Global analysis using wavelets */
				Wavelet_exec(waveletSignal,n_points,&sizeSig2);
				
				#pragma omp taskwait
				
			}
			else
			{
				counter=2;
			}
		}
		
		free(signal);
		if(signal!=signal3) free(signal3);
		free(waveletSignal);
		
		fprintf(hp, "\n\n");
		#ifdef TRACE_MODE
			Extrae_event (1000, 0);
		#endif
		
	}
//#pragma css finish
/* finish SMPSs */

	fprintf(hp, "Speedup Analysis\n");
	for(trace_num=1; trace_num < argc-1; trace_num++)
	{
		fprintf(hp, "Trace %d : %f\n", trace_num, (1.0*periods[1])/(1.0*periods[trace_num]));

	}

	fclose(hp);
	fclose(jp);

	//system("rm *.txt trace.*.filtered.prv trace.*.filtered.pcf PPP2 KK Tcorrect");

	printf("Success!!!");

	//for test.sh
	#ifndef TEST_MODE
		temps = time(NULL) - temps;
		printf("%li Hours, %li Minutes, %li Seconds\n", temps/3600, (temps%3600)/60,(temps%3600)%60);
	#endif
	#ifdef TRACE_MODE
		Extrae_user_function(0);
		Extrae_fini();
	#endif
	return 0;
}
#endif
