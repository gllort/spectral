#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "optim-macros.h"


int compare (const void * a, const void * b)
{
  if(*(long long int*)a > *(long long int*)b) return 1;
  if(*(long long int*)a==*(long long int*)b) return 0;
  if(*(long long int*)a < *(long long int*)b) return -1;
  
  /*return ( *(long long int*)a - *(long long int*)b );*/
}



float Maxim(long long int p, long long int *t, long long int *delta, float *semanticvalue, int i, long long int llargint) {

int j, k;
float maxim, b;

p=p+1;
j=0;
while (j<i & p>=t[j]) {
	j++;
}
k=MAX(0, j-1);
maxim=semanticvalue[k];
k=k-1;
while(k>-1 & (llabs(t[k]+delta[k]-p)<=llargint/2)) {
	b=semanticvalue[k];
	maxim=MAX(b, maxim);
	k--;
}
k=j;
while(k<i-1 & (llabs(t[k]-p)<=llargint/2)) {
	b=semanticvalue[k];
	maxim=MAX(b, maxim);
	k++;
}
return maxim;
}


void Passadamax(long long int *t, long long int *delta, float *semanticvalue, int i, long long int *B1, float *B2, long long int longint) {

int j, k, count;



k=0;
j=0;

B1[k]=t[j];
k++;

B1[k]=t[j]-longint/2;
k++;

B1[k]=t[j]+longint/2;
k++;

for(j=1; j<i; j++) {
	B1[k]=t[j]-longint/2;
	k++;
	
	B1[k]=t[j];
	k++;
	
	B1[k]=t[j]+longint/2;
	k++;
}

B1[k]=t[j-1]+delta[j-1];

/*ordenacio*/


qsort(B1, 3*i+1, sizeof(long long int), compare);

for(j=0; j<3*i+1; j++) {
	B2[j]=Maxim(B1[j], t, delta, semanticvalue, i, longint);
}


}

int SenyalD(long long int *t, long long int *delta, float *semanticvalue, int i, long long int longint, long long int *t2, long long int *delta2, float *semanticvalue2) {
	long long int *B1, *temp1, *P1;
	float *B2, *temp2, *P2;
	int k, j, sizeB, sizeP, sizeM, count;
	
	k=1;
	j=1;
	sizeB=3*i+1;
	
	
	B1 = (long long int *)malloc(sizeof(long long int)*sizeB);
	B2 = (float *)malloc(sizeof(float)*sizeB);
	
	temp1 = (long long int *)malloc(sizeof(long long int)*sizeB);
	temp2 = (float *)malloc(sizeof(float)*sizeB);
	
	Passadamax(t, delta, semanticvalue, i, B1, B2, longint);
	
	k=0;
	j=0;
	
	while (j<=sizeB-2) {
		temp1[k]=B1[j];
		temp2[k]=B2[j];
		k++;
		j++;
		
		if (B2[j]==B2[j-1]) {
			while (B2[j]==B2[j-1] & j<=sizeB-2) {
				j++;
			}
			temp1[k]=B1[j-1];
			temp2[k]=B2[j-1];
			k++;
		}
	}
	if (j==sizeB-1) {
		temp1[k]=B1[j];
		temp2[k]=B2[j];
		k++;
	}
	
	
	P1 = (long long int *)malloc(sizeof(long long int)*2*k);
	P2 = (float *)malloc(sizeof(float)*2*k);
	
	for(j=1; j<=k; j++) {
		P1[2*j-2]=temp1[j-1];
		P2[2*j-2]=temp2[j-1];
		P1[2*j-1]=temp1[j-1];
		P2[2*j-1]=Maxim(temp1[j-1]+1, t, delta, semanticvalue, i, longint);
	}
	sizeP=2*k;
	sizeM=2*k+1;
	j=0;
	k=0;
	/*t2 = (long long int *)malloc(sizeof(long long int)*sizeM);
	delta2 = (long long int *)malloc(sizeof(long long int)*sizeM);
	semanticvalue2 = (float *)malloc(sizeof(float)*sizeM);*/
	

	t2[k]=P1[j]-0.00001;
	delta2[k]=0.00001;
	semanticvalue2[k]=0;
	k++;
	
	for(j=0; j<sizeP-1; j++) {
		t2[k]=P1[j];
		delta2[k]=P1[j+1]-P1[j];
		//printf("%lld %lld %lld %lld\n", P1[j+1], P1[j], t2[k], delta2[k]);
		//printf("3 %d %lld %lld %f\n", k, t2[3], delta2[3], semanticvalue2[3]);
		semanticvalue2[k]=P2[j];
		if (delta2[k]!=0) {
			k++;
		}
		/*j++;*/
	}
	k++;
	//printf("3 %d %lld %lld %f\n", k, t2[3], delta2[3], semanticvalue2[3]);
	//t2[k]=t2[k-1]+delta2[k-1];
	//delta[k]=0;
	//semanticvalue[k]=0;
	//k++;
	//printf("3 %d %lld %lld %f\n", k, t2[3], delta2[3], semanticvalue2[3]);
	
	/*for (count = 0; count < k-1; count++) {
		printf("%d %lld %lld %f\n", count, t2[count], delta2[count], semanticvalue2[count]);
  	} */ 
	
	free(B1);
	free(B2);
	free(temp1);
	free(temp2);
	free(P1);
	free(P2);
	return k;
}



int main(int argc, char *argv[]) 
{ 
 
  long int a, b, i, j, prova, sizeM;
  float c;
  FILE *fp;
  long long int *t, *delta, *t2, *delta2, longme;
  float *semanticvalue, *semanticvalue2;
  char filename[256];
  int count;
  
  if (argc!=4) {
	printf("Usage: %s <input datafile> <Ero/Dil function size> <output datafile>\n", argv[0]);
	exit (66);
  }
  strcpy(filename, argv[1]);
  /*longme=atoi(argv[2]);
  longme=1000000000;
  printf("%lld\n", longme);*/
  longme=strtoll(argv[2], NULL, 10); 
  //printf("%lld\n", longme);
  fp = fopen(filename, "r");
  if (fp==NULL) {
  	printf("file not open\n");
  	exit (77);
  }
  //printf("1\n");
  i=0;
  while (fscanf(fp, "%ld %ld %f", &a, &b, &c)==3) {
  	i++;	
  }
  //printf("Numero de files: %ld \n", i);

  prova=fseek(fp, 0L, SEEK_SET);
  
  t = (long long int *)malloc(sizeof(long long int)*i);
  delta = (long long int *)malloc(sizeof(long long int)*i);
  semanticvalue = (float *)malloc(sizeof(float)*i);
  
  t2 = (long long int *)malloc(sizeof(long long int)*6*i+3);
  delta2 = (long long int *)malloc(sizeof(long long int)*6*i+3);
  semanticvalue2 = (float *)malloc(sizeof(float)*6*i+3);
  
  j=0;
  while (fscanf(fp, "%lld %lld %f", &t[j], &delta[j], &semanticvalue[j])==3) {
   	j++;
  }
  
  sizeM=SenyalD(t, delta, semanticvalue, i, longme, t2, delta2, semanticvalue2);
  //printf("2\n"); 
  /*for (j = 0; j < sizeM; j++) {
                printf("%lld %lld %f\n", t2[j], delta2[j], semanticvalue2[j]);
  }*/
  
  fclose(fp);
  
  /*memcpy(str1, "dil", 4);
  memcpy(str1+3, argv[2], strlen(argv[2])+1);
  memcpy(str1+3+strlen(argv[2]), filename, strlen(filename)+1);*/
  strcpy(filename, argv[3]);
  fp=fopen(filename, "w");
  for (j = 0; j < sizeM; j++) {
                fprintf(fp, "%lld %lld %f\n", t2[j], delta2[j], semanticvalue2[j]);
  }
  fclose(fp);
  /*free(t);
  free(delta);
  free(semanticvalue);*/
  return 0;
}
