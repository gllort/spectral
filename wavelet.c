#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>



typedef struct {
  long long int t;
  double val;
} block;

int compare (const void * a, const void * b)
{
  if( ((block*)a)->val < ((block*)b)->val) return 1;
  if( ((block*)a)->val == ((block*)b)->val) return 0;
  if( ((block*)a)->val > ((block*)b)->val) return -1;

  /*return ( *(long long int*)a - *(long long int*)b );*/
}


void haartransform(double*input, double*output, int n) {

	int k, kk;
	double i, length, sum, dif, prova;
	long long int a;

	for(k=0; k<n; k++) {
		output[k]=0.0;
	}
	
	a=1;
	i=log(n*1.0)/log(2);
	
	a=exp((i-1)*log(2));
	
	for(length=i; length>0; length--) {
		prova=(exp((length-1)*log(2)));
		for(k=1; k<=prova; k++) {
			sum=input[k*2-2]+input[k*2-1];
			dif=input[k*2-2]-input[k*2-1];
			output[k-1]=sum;
		        output[k+a-1]=dif;
		}
		for(kk=0; kk<n; kk++) {
			input[kk]=output[kk];
		}

		
		a=a/2;
		
	}
		

}
	
	



int main (int argc, char *argv[]) {

	double *input;
	double *output, a, i, freq, MAXvalue, SUM, DEV;
	long long int n, m, k, j, MAX, MIN, MAX2, MIN2, MAX3, MIN3, tini;
	FILE *fp;
	block *result, *maximum;
	long long int results[5][82];
	
	if (argc!=3 && argc!=4) {
		printf("Usage: %s <input datafile> <output datafile> <frequency (optional)>\n", argv[0]);
		exit (0);
  	}
	if(argc==4) { 
		freq=atof(argv[3]);
	}
	


  	if((fp = fopen(argv[1], "r")) == NULL)
  	{
        	printf("\nWavelet: Can't open file %s !!!\n\n", argv[1]);
        	exit(-1);
  	}



	n=0;
	while (fscanf(fp, "%lf\n", &a)!= EOF) {
  		n++;	
  	}

	MIN=n;
	MIN2=n;
	MIN3=n;
	MAX=0;
	MAX2=0;
	MAX3=0;
	
	fseek(fp, 0L, SEEK_SET);
	
	input = (double*)malloc(sizeof(double)*n);
	output = (double*)malloc(sizeof(double)*n);
	result = (block *)malloc(sizeof(block)*n);
	maximum = (block *)malloc(sizeof(block)*n);
	
	j=0;
	while (fscanf(fp, "%lf", &input[j])==1) {
  		j++;	
  	}
	
	fclose(fp);
	
	haartransform(input, output, n);

	SUM=0;

        for(k=0; k<n/2; k++) {
                result[k].t=k;
                result[k].val=fabs(output[k+n/2]);
                SUM=SUM+result[k].val;
        }

        SUM=SUM*2/n;

        for(k=0; k<n/2; k++) {

                DEV=DEV+(SUM-result[k].val)*(SUM-result[k].val);

        }

        DEV=DEV*2/n;

        DEV=sqrt(DEV);

        //printf("Exp Val=%lf, Dev=%lf\n", SUM, DEV);

        //printf("Boundary=%lf\n", SUM+5*DEV);

	fp = fopen(argv[2], "w");


	MAXvalue=0;	
	for(k=0; k<n/2; k++) {
		if(result[k].val>MAXvalue) {	
			MAXvalue=result[k].val;
		}
	}

	MAXvalue=SUM+5*DEV;

	//printf("%lf\n", MAXvalue);
	

	i=1;
	k=pow(2,i);
		//printf("Level %lf. Height=%lf\n", i+1, (1.0*k*MAXvalue)/(1.0*16));
		MAX=result[0].t;
		for(m=0; m<n/2; m++) {
			if((1.0*k*MAXvalue)/(1.0*16)<=result[m].val) {
				MAX=result[m].t;
				fprintf(fp, "%lf\n", (1.0*result[m].t)/(1.0*n/2));
				tini=result[m].t;
				while(((1.0*k*MAXvalue)/(1.0*16)<=result[m].val ||
				MAX+50>result[m].t) && m<n/2) {
					if((1.0*k*MAXvalue)/(1.0*16)<=result[m].val) {
						MAX=result[m].t;
					}
					m++;
					//printf("tdins=%lld\n", result[m].t);
				}
				fprintf(fp, "%lf\n", (1.0*MAX)/(1.0*n/2));
			}
		}		
		i++;

	
	fclose(fp);
	
	return 0;
	

}


