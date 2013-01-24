#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>
#include <strings.h>


int compare (const void * a, const void * b)
{
	if(*(double*)a > *(double*)b) return 1;
	if(*(double*)a==*(double*)b) return 0;
	if(*(double*)a < *(double*)b) return -1;
}


int main(int argc, char *argv[])
{
	fftw_complex *in=NULL, *out=NULL;
	fftw_plan p;
	int N, i, j, freq;
	FILE *fp=NULL, *gp=NULL;
	double prova, *conj=NULL, *conj2=NULL;

	if(argc!=4) {
		printf("Usage:%s <input file> <output file> <freq of sampling>\n", argv[0]);
		exit(0);
	}
	fp=fopen(argv[1], "r");
	gp=fopen(argv[2], "w");
	freq=atoi(argv[3]);
	i=0;
	while(fscanf(fp, "%lf ", &prova)==1) {
                i++;
        }
	N=i;
	fseek(fp, 0L, SEEK_SET);
	in=fftw_malloc(sizeof(fftw_complex)*N);
	bzero(in, sizeof(fftw_complex)*N);
	out=fftw_malloc(sizeof(fftw_complex)*N);
	bzero(out, sizeof(fftw_complex)*N);
	conj=(double *)malloc(sizeof(double)*N);
	bzero(conj, sizeof(double)*N);
	conj2=(double *)malloc(sizeof(double)*N);
	bzero(conj2, sizeof(double)*N);
	p=fftw_plan_dft_1d(N, in, out, -1, FFTW_ESTIMATE);
	for(i=0; i<N; i++) {
		fscanf(fp, "%lf ", &in[i]);
	}
	fftw_execute(p);
	for(i=0; i<N; i++) {
		conj[i]=pow(cabs(out[i]),2)/N;
		fprintf(gp, "%lf\n", conj[i]);
		/*printf("%f %lf\n", (1.0*freq*i)/(1.0*N), conj[i]);*/
	}
	/*fftw_destroy_plan(p);
	fftw_free(in); fftw_free(out);*/
	fclose(fp);
	fclose(gp);
}

