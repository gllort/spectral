#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "optim-macros.h"
#include "SenyalD.h"

int compare_SenyalD (const void * a, const void * b)
{
	if(*(long long int*)a > *(long long int*)b) return 1;
	else if(*(long long int*)a==*(long long int*)b) return 0;
	else return -1;

	/*return ( *(long long int*)a - *(long long int*)b );*/
}


float Maxim(long long int p, struct signal_info *signal, long int i, long long int llargint)
{

	int j, k;
	float maxim;

	p=p+1;
	j=0;
	while (j<i && p>=signal[j].t)
	{
		j++;
	}
	k=MAX(0, j-1);
	maxim=signal[k].semanticvalue;
	k=k-1;
	while(k>-1 && (llabs(signal[k].t+signal[k].delta-p)<=llargint))
	{
		maxim=MAX(signal[k].semanticvalue, maxim);
		k--;
	}
	k=j;
	while(k<i-1 && (llabs(signal[k].t-p)<=llargint))
	{
		maxim=MAX(signal[k].semanticvalue, maxim);
		k++;
	}
	return maxim;
}


void Passadamax(struct signal_info *signal, long int i, long long int *B1, float *B2, long long int longint)
{

	int j, k;

	B1[0]=signal[0].t;
	B1[1]=signal[0].t-longint;
	B1[2]=signal[0].t+longint;
	
	k=3;
	for(j=1; j<i; j++)
	{
		B1[k]=signal[j].t-longint;
		B1[k+1]=signal[j].t;
		B1[k+2]=signal[j].t+longint;
		k+=3;
	}

	B1[k]=signal[j-1].t+signal[j-1].delta;

	/*ordenacio*/

	qsort(B1, 3*i+1, sizeof(long long int), compare_SenyalD);

	for(j=0; j<3*i+1; j++)
	{
		B2[j]=Maxim(B1[j], signal, i, longint);
	}

}


int SenyalD_exec(struct signal_info *signal, long long int longint, long int size, struct signalFloat_info *signal2)
{
	
	/**
	* pre:
	*	filename: input datafile
	*	longme: Ero/Dil function size
	*	size: size file
	*	signal2: outdata
	* post:	
	*	signal2: output data
	*	@return: size outputdata
	**/
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	long int j;
	long long int *B1, *temp1, *P1;
	float *B2, *temp2, *P2;
	int k, sizeB, sizeP;
	
	sizeB=3*size+1;

	B1 = (long long int *)malloc(sizeof(long long int)*sizeB);
	B2 = (float *)malloc(sizeof(float)*sizeB);

	temp1 = (long long int *)malloc(sizeof(long long int)*sizeB);
	temp2 = (float *)malloc(sizeof(float)*sizeB);

	Passadamax(signal,size, B1, B2, longint/2);

	k=0;
	j=0;

	while (j<=sizeB-2)
	{
		temp1[k]=B1[j];
		temp2[k]=B2[j];
		k++;
		j++;

		if (B2[j]==B2[j-1])
		{
			while (B2[j]==B2[j-1] && j<=sizeB-2)
			{
				j++;
			}
			temp1[k]=B1[j-1];
			temp2[k]=B2[j-1];
			k++;
		}
	}
	if (j==sizeB-1)
	{
		temp1[k]=B1[j];
		temp2[k]=B2[j];
		k++;
	}

	P1 = (long long int *)malloc(sizeof(long long int)*2*k);
	P2 = (float *)malloc(sizeof(float)*2*k);

	for(j=1; j<=k; j++)
	{
		P1[2*j-2]=temp1[j-1];
		P2[2*j-2]=temp2[j-1];
		P1[2*j-1]=temp1[j-1];
		P2[2*j-1]=Maxim(temp1[j-1]+1, signal, size, longint/2);
	}
	sizeP=2*k;
	
	signal2[0].t=P1[0]-0.00001;
	signal2[0].delta=0.00001;
	signal2[0].semanticvalue=0;
	k=1;

	for(j=0; j<sizeP-1; j++)
	{
		signal2[k].t=P1[j];
		signal2[k].delta=P1[j+1]-P1[j];
		signal2[k].semanticvalue=P2[j];
		if (signal2[k].delta!=0)
		{
			k++;
		}
	}
	k++;
	
	free(B1);
	free(B2);
	free(temp1);
	free(temp2);
	free(P1);
	free(P2);
	#ifdef DEBUG_MODE
		FILE *fp;
		if((fp = fopen("out.txt", "w")) == NULL)
		{
			printf("\nDebug: Can't open file out.txt !!!\n\n");
			exit(-1);
		}
		for(j=0;j<k;j++)
		{
			fprintf(fp, "%lld %lld %f\n", signal2[j].t, signal2[j].delta, signal2[j].semanticvalue);
		}
		fclose(fp);
	#endif
		
	#ifdef TRACE_MODE
		Extrae_user_function(0);
	#endif
	return k;
}
