#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <complex.h>
#include <fftw3.h>
#include "optim-functions.h"
#include "fft.h"

void fft_exec(fftw_complex *in,int N,struct signalFreq_info *conj, long long int freq)
{
	/**
	*pre:
	*	input: input file
	*	output: output file
	*	freq: freq of sampling
	**/
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	fftw_complex *out;
	fftw_plan p;
	int i;	
	#ifdef DEBUG_MODE
		FILE *fDB1;
		fDB1 = fopen("Freq2x.txt", "w");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file Freqx.txt !!!\n\n");
			exit(-1);
		}
	#endif
	
	out=fftw_malloc(sizeof(fftw_complex)*N);
	bzero(out, sizeof(fftw_complex)*N);
		
	p=fftw_plan_dft_1d(N, in, out, -1, FFTW_ESTIMATE);
	
	fftw_execute(p);
	for(i=0; i<N; i++)
	{
		conj[i].freq=1.0*i*freq;
		conj[i].conj=pow(cabs(out[i]),2)/pow(N,3);
		#ifdef DEBUG_MODE
			fprintf(fDB1, "%lf %lf\n", conj[i].freq,conj[i].conj);
		#endif
		//fprintf(gp, "%lf %lf\n", 1.0*i*freq, conj[i]);
		/*fprintf(gp ,"%f %lf\n", (1.0*freq*i)/(1.0*N), conj[i]);*/
	}
	
	fftw_destroy_plan(p);
	fftw_free(out);
	#ifdef DEBUG_MODE
		fclose(fDB1);
	#endif
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}