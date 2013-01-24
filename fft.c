#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>


int compare (const void * a, const void * b)
{
	if(*(double*)a > *(double*)b) return 1;
	if(*(double*)a==*(double*)b) return 0;
	if(*(double*)a < *(double*)b) return -1;
}


int main(int argc, char *argv[])
{
	fftw_complex *in, *out;
	fftw_plan p;
	int N, i, j, freq;
	FILE *fp, *gp;
	double prova, *conj, *conj2;

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
	//printf("%d\n", i);
	fseek(fp, 0L, SEEK_SET);
	in=fftw_malloc(sizeof(fftw_complex)*N);
	out=fftw_malloc(sizeof(fftw_complex)*N);
	conj=(double *)malloc(sizeof(double)*N);
	conj2=(double *)malloc(sizeof(double)*N);
	/*p=fftw_plan_dft_1d(N, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);*/
	p=fftw_plan_dft_1d(N, in, out, -1, FFTW_ESTIMATE);
	for(i=0; i<N; i++) {
		fscanf(fp, "%lf ", &in[i]);
	}
	fftw_execute(p);
	for(i=0; i<N; i++) {
		conj[i]=pow(cabs(out[i]),2)/pow(N,3);
		fprintf(gp, "%lf %lf\n", 1.0*i*freq, conj[i]);
		/*fprintf(gp ,"%f %lf\n", (1.0*freq*i)/(1.0*N), conj[i]);*/
	}
	/*
	for(i=0; i<N; i++) {
		conj2[i]=conj[i];
	}
	qsort(conj2, N/2, sizeof(double), compare);
	for(i=N/2-1; i>N/2-11; i--) {
		fprintf(gp, "%lf ", conj2[i]);
		j=0;
		while(conj[j]!=conj2[i]) {
			j++;	
		}
		fprintf(gp, "%f\n", (1.0*freq*j)/(1.0*N));
		fprintf(gp, "%d\n", j);
	}
	fftw_destroy_plan(p);
	fftw_free(in); fftw_free(out);*/
	fclose(fp);
	fclose(gp);
}

