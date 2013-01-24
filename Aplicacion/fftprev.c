#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>
#include "fftprev.h"


void fftprev_exec(fftw_complex *in,int N ,fftw_complex *out)
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
	fftw_plan p;
	int i;
	#ifdef DEBUG_MODE
		FILE *fDB1;
		fDB1 = fopen("Freqx.txt", "w");
		if (fDB1==NULL)
		{
			printf("\nDebug: Can't open file Freqx.txt !!!\n\n");
			exit(-1);
		}
	#endif
		
	p=fftw_plan_dft_1d(N, in, out, -1, FFTW_ESTIMATE);
	
	fftw_execute(p);
	for(i=0; i<N; i++)
	{
		out[i]=pow(cabs(out[i]),2)/N;
		/*
		out[i][0]=pow(cabs(out[i]),2)/N; //real --> cabs need complex.h
		out[i][1]=0; //imaginary
		/*
		conj=pow(cabs(out[i]),2)/N;
		bzero(&out[i], sizeof(fftw_complex));
		out[i]=conj;
		*/
		#ifdef DEBUG_MODE
			fprintf(fDB1, "%lf\n", out[i]);
		#endif
	}
	fftw_destroy_plan(p);
	
	#ifdef DEBUG_MODE
		fclose(fDB1);
	#endif
		
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
}