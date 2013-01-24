#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SenyalE.h"
#include "optim-macros.h"

int compare_SenyalE (const void * a, const void * b)
{
	if(*(long long int*)a > *(long long int*)b) return 1;
	else if(*(long long int*)a==*(long long int*)b) return 0;
	else return -1;

	/*return ( *(long long int*)a - *(long long int*)b );*/
}


float Minim(long long int p, struct signalFloat_info *signal, int i, long long int llargint)
{

	int j, k;
	float minim;

	p=p+1;
	llargint=llargint-1;
	j=0;
	while (j<i & p>=signal[j].t)
	{
		j++;
	}
	k=MAX(0, j-1);
	minim=signal[k].semanticvalue;
	k=k-1;
	while(k>-1 & (llabs(signal[k].t+signal[k].delta-p)<=llargint))
	{
		minim=MIN(signal[k].semanticvalue, minim);
		k--;
	}
	k=j;
	while(k<i-1 & (llabs(signal[k].t-p)<=llargint))
	{
		minim=MIN(signal[k].semanticvalue, minim);
		k++;
	}
	return minim;
}


void Passadamin(struct signalFloat_info *signal, int i, long long int *B1, float *B2, long long int longint)
{

	int j, k;

	B1[0]=signal[0].t;
	k=1;

	for(j=1; j<i; j++)
	{
		B1[k]=signal[j].t-longint;
		B1[k+1]=signal[j].t;
		B1[k+2]=signal[j].t+longint;
		k+=3;
	}

	B1[k]=signal[j-1].t+signal[j-1].delta;

	/*ordenacio*/

	qsort(B1, 3*i-1, sizeof(long long int), compare_SenyalE);

	for(j=0; j<3*i-1; j++)
	{
		B2[j]=Minim(B1[j], signal, i, longint);
	}
}


//int SenyalE( int i, long long int longint, long long int *t2, long long int *delta2, float *semanticvalue2)
//     return SenyalE(t, delta, semanticvalue, size, longme, t2, delta2, semanticvalue2);
//int SenyalE_exec(long long int longme,long int size,long long int *t, long long int *delta, float *semanticvalue, long long int *t2, long long int *delta2, float *semanticvalue2);
int SenyalE_exec(long long int longint,long int i,struct signalFloat_info *signal, struct signalFloat_info *signal2)
{
	/**
	* pre:
	*	t,delta,sematicvalue: input data
	*	longint: Ero/Dil function size
	*	t2,delta2,sematicvalue2: output data
	* post:	
	*	t2,delta2,sematicvalue2: output data
	*	@return: size outputdata
	**/
	#ifdef TRACE_MODE
		Extrae_user_function(1);
	#endif
	long long int *B1, *temp1, *P1;
	float *B2, *temp2, *P2;
	int k, j, sizeB, sizeP;

	sizeB=3*i-1;

	B1 = (long long int *)malloc(sizeof(long long int)*sizeB);
	B2 = (float *)malloc(sizeof(float)*sizeB);

	temp1 = (long long int *)malloc(sizeof(long long int)*sizeB);
	temp2 = (float *)malloc(sizeof(float)*sizeB);

	Passadamin(signal, i, B1, B2, longint/2);

	k=0;
	j=0;

	while (j<sizeB)
	{
		temp1[k]=B1[j];
		temp2[k]=B2[j];
		k++;
		j++;

		if (B2[j]==B2[j-1])
		{
			while (B2[j]==B2[j-1] & j<=sizeB-2)
			{
				j++;
			}
			temp1[k]=B1[j-1];
			temp2[k]=B2[j-1];
			k++;
		}
	}
	
	P1 = (long long int *)malloc(sizeof(long long int)*2*k);
	P2 = (float *)malloc(sizeof(float)*2*k);

	for(j=1; j<=k; j++)
	{
		P1[2*j-2]=temp1[j-1];
		P2[2*j-2]=temp2[j-1];
		P1[2*j-1]=temp1[j-1];
		P2[2*j-1]=Minim(temp1[j-1]+1, signal, i, longint/2);
	}
	sizeP=2*k;
	j=0;
	k=0;
	
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
	
	free(B1);
	free(B2);
	free(temp1);
	free(temp2);
	free(P1);
	free(P2);
	
	#ifdef DEBUG_MODE
		FILE *fp;
		if((fp = fopen("out2.txt", "w")) == NULL)
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
