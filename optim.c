#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#if defined (GS_VERSION) 
	#include <GS_master.h>
#endif
#include "optim.h"
#include "optim-macros.h"
#include "optim-functions.h"


#define STEPS 10

#define STEPSbis 2
#define ITERS 4 



void ini_rand()
{
long int t;

t = time(NULL);
srandom((unsigned int) t);
}



int Nthreads(file dades)
{

FILE *fp;
char s[10000];
int p;

fp=fopen(dades, "r");

fgets(s, 10000, fp);

sscanf(s, "%*s %*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%d", &p);

fclose(fp);

return p;
}



long long int Totaltime(file dades)
{

FILE *fp;
char s[100];
long long int t;


fp=fopen(dades, "r");

//fp = GS_FOPEN(dades, R);

if(fp==NULL) {
	printf("Error loading file\n");
	exit(-2);
}

fgets(s, 100, fp);

sscanf(s, "%*s %*[^:]:%*[^:]:%lld", &t);

//GS_FCLOSE(fp);

fclose(fp);

return t;
}




void getValues(file filename, long long int *t) {

float sem;
long long int time, delta;
int i = 0;
FILE *fp;


GS_FOPEN(fp,filename, R);

while(fscanf(fp, "%lld %lld %f\n", &time, &delta, &sem) == 3) {
	if (sem!=0.0) {
		t[i]=time;
		i++;
		t[i]=time+delta;
		while((fscanf(fp, "%lld %lld %f\n", &time, &delta, &sem) == 3) && (sem!=0.0)) {
			t[i]+=delta;
		}
		i++;
	}
}
GS_FCLOSE(fp);
t[i]=-1;

}



void GetBoundary(file trace, file output, long long int totaltime) {

long long int k, kant, prova, t[1000];
int j, i, flush;
FILE *fp, *gp;


j=0;


GS_FOPEN(fp,trace, R);
if(fscanf(fp, "%lld %lld %lld", &k, &kant, &prova)==3) {
	flush=1;
}
else {
	flush=0;
}
GS_FCLOSE(fp);

k=1000000;


if(flush==1) {
	GS_FOPEN(gp,output, W);
	//k=totaltime * 0.01;
	k=totaltime * 0.001;
	GS_FOPEN(fp,"KK", W);
	fprintf(fp, "%lld", k);
	GS_FCLOSE(fp);
	SenyalD(trace, k, "out.txt");
	SenyalE("out.txt", k, "out2.txt");
	getValues("out2.txt", t);
	i=0;
	while(t[i]!=-1) {
		/*printf("Flushing from %ld to %ld ms\n", t[i]/1000000, t[i+1]/1000000);*/
		fprintf(gp, "%lld %lld\n", t[i], t[i+1]);
		i=i+2;
	}
	GS_FCLOSE(gp);
fprintf(stderr, "t size=%d\n", i);
	
	
}
else {
	GS_FOPEN(gp,output, W);
	fprintf(gp, "0\n");
	GS_FCLOSE(gp);
}


}


long long int Maximum(file signal)
{

FILE *fp;
long long int max, maxt, maxdelta, t, delta;
double value;

GS_FOPEN(fp,signal, R);
max=0;
maxt=0;
maxdelta=0;
while(fscanf(fp, "%lld %lld %lf", &t, &delta, &value)==3) {
	if(value>max) {
		max=value;
		maxt=t;
		maxdelta=delta;
	}
}

GS_FCLOSE(fp);
return (maxt+maxdelta/2);

}
		

void GetPeriod(file signal, int Freqx, int j, long long int delta, long long int ampli, file Tcorrect, char* option, char *tmp_dirname)
{
	FILE *fp;
	STRING(signalx2_txt); STRING(KKx); STRING(tmpx_txt); STRING(signalxx_txt);
	STRING(px_txt); STRING(Freqx_txt); STRING(Freq2x_txt); STRING(output_txt);

	TMPFILE(signalx2_txt, tmp_dirname, "signalx2.txt");
	TMPFILE(KKx, tmp_dirname, "KKx");
	TMPFILE(tmpx_txt, tmp_dirname, "tmpx.txt");
	TMPFILE(signalxx_txt, tmp_dirname, "signalxx.txt");
	TMPFILE(px_txt, tmp_dirname, "px.txt");
	TMPFILE(Freqx_txt, tmp_dirname, "Freqx.txt");
	TMPFILE(Freq2x_txt, tmp_dirname, "Freq2x.txt");
	TMPFILE(output_txt, tmp_dirname, "output.txt");

	Correction(signal, delta, signalx2_txt);
	if(j==1) {
		if(ampli!=0) {
			GS_FOPEN(fp, KKx, W);
			fprintf(fp, "%lld", ampli);
			GS_FCLOSE(fp);
			SenyalD(signalx2_txt, ampli, tmpx_txt);
			SenyalE(tmpx_txt, ampli, signalxx_txt);
			Sampler(signalxx_txt, px_txt, (long long int)Freqx);
		}
		else {
			Sampler(signalx2_txt, px_txt, (long long int)Freqx);
		}
	}
	else {
		Sampler2(signalx2_txt, px_txt);
	}

	Fftprev(px_txt, Freqx_txt, Freqx);
	Fft(Freqx_txt, Freq2x_txt, Freqx);
	GetTime(Freq2x_txt, signalx2_txt, j, Tcorrect, output_txt);
}



void Generatesinus(long long int span, long long int T2, long long int acc, file sinus, float num_iters) {


	FILE *fp;
	long long int i;
	long long int sinus_periods = 4;


	GS_FOPEN(fp, sinus, W);
/*
        if(num_iters < 30)
		sinus_periods = 2;
	else
		sinus_periods = 4;
*/	

	for(i=0; i*acc<sinus_periods*T2; i++) {
		fprintf(fp, "%lf\n", sin(2*(3.1416)/T2*i*acc)+1 );
		
	}
	
	for(i=sinus_periods*T2/acc; i*acc<span; i++) {
		fprintf(fp, "0\n");
	}
	
	GS_FCLOSE(fp);
	
	
}


Period_t * Analysis(file signal, long long int t0, long long int t1, long int duration, long int r0, long int r1,
int n, int i, int j2, char* option, file trace, int cut, int p, file signal2, file signal3, int* pfound, 
long int* period, file signalout, long int* point1, long int* point2, file filename, FILE *out,
FILE *err, int num_chop, char *tmp_dirname, int requested_iters) {

	long long int min, min2, T2, T3, Tcorr, amplitud, Tprev, spectral_trace_size;
	long long int amplicorr, size, size2, ampliprev, max, accuracy;
	double goodness, goodcorr, goodness2, goodcorr2, goodness3, goodcorr3, goodprev, goodprev2, goodprev3;
	int j, correct, k, count, zigazaga, nzeros, zigazagacorr, nzeroscorr, zigazagaprev, nzerosprev, n_iters;
	char filename2[256], filename4[256], *env_var;
	FILE *fp, *gp, *pFile, *pFile2;
	Period_t * currentPeriod = NULL;
	STRING(outin_txt); STRING(outin_samp_txt); STRING(Tcorrect); STRING(sin_txt); STRING(Crosscorrelation_txt); STRING(signalt_txt);
	
  fprintf(stderr, "\n\n ANALYSIS -- signal1=%s t0=%lld t1=%lld duration=%lld r0=%lld r1=%lld option=%s trace=%s cut=%d signal3=%s num_chop=%d req_iters=%d\n\n",
signal,
t0,
t1,
duration,
r0,
r1,
option,
trace,
cut,
signal3,
num_chop,
requested_iters);


	//fprintf(stderr, "flushing=%d, recursion=%d, pzone=%d\n", n, i, j2);

	TMPFILE(outin_txt, tmp_dirname, "outin.txt");
	TMPFILE(outin_samp_txt, tmp_dirname, "outin.samp.txt");
	TMPFILE(Tcorrect, tmp_dirname, "Tcorrect");
	TMPFILE(sin_txt, tmp_dirname, "sin.txt");
	TMPFILE(Crosscorrelation_txt, tmp_dirname, "Crosscorrelation.txt");
	TMPFILE(signalt_txt, tmp_dirname, "signalt.txt");

	//fprintf(stderr, "%lld %lld %lld %lld\n", t0, t1, t0*1000000, t1*1000000);
	Cutter_signal(signal, outin_txt, t0*1000000, t1*1000000);

        min=t0*1000000;
        correct=0;
	count=0;
        amplitud=0;
        T2=1000000;
	Tcorr=-1;
	zigazaga=0;
	goodcorr=0;
	
	/* Looking for the main period in outin.txt signal. Condicio correct==0 afegida!!*/
	
	//accuracy=(t1-t0)/500 * 1000000;
	accuracy=500000;
	while((T2/1000000!=0 && zigazaga==0 && T2/1000000<(t1-t0)/3.5 && correct==0) || (T2/1000000!=0 &&
	T2/1000000<(t1-t0)/3.5 &&(1.0*(t1-t0))/(1.0*T2/1000000)>300)) {
                GetPeriod(outin_txt, accuracy, 1, t1-t0, amplitud, Tcorrect, option, tmp_dirname);
		// Accuracy = 5000000 usualment !!!!
                GS_FOPEN(fp,Tcorrect, R);
                Tprev=T2;
                ampliprev=amplitud;
                goodprev=goodness;
                goodprev2=goodness2;
                goodprev3=goodness3;
		zigazagaprev=zigazaga;
		nzerosprev=nzeros;
                fscanf(fp, "%lld %d %lf %lf %lf %d %d", &T2, &correct, &goodness, &goodness2, &goodness3, &zigazaga, &nzeros);
                GS_FCLOSE(fp);
                if(((correct==1 && goodcorr<goodness) || (zigazaga==1 && goodcorr < goodness))&&(1.0*(t1-t0))/(1.0*T2/1000000)>2.5) {

                                Tcorr=T2;
                                amplicorr=amplitud;
				goodcorr=goodness;
				goodcorr2=goodness2;
				goodcorr3=goodness3;
				zigazagacorr=zigazaga;
				nzeroscorr=nzeros;
                }
  		fprintf(err, "T2=%lld ns\n", T2);
                fprintf(err, "Accuracy=%lld ns\n", accuracy);
                fprintf(err, "Iters=%f, correct=%d, goodness=%lf, goodness2=%lf, goodness3=%lf, zz=%d, nz=%d\n", (1.0*(t1-t0))/(1.0*T2/1000000), correct, goodness, goodness2, goodness3, zigazaga, nzeros);
                //amplitud=amplitud+T2/10;
		accuracy=accuracy+T2/10;
		count++;
        }
	fprintf(stderr, "T2=%lld zigazaga=%d (t1-t0)/2=%ld correct=%d\n", T2, zigazaga, (t1-t0)/2, correct);
	if(Tcorr!=-1) {
		amplitud=amplicorr;
        	T2=Tcorr;
		goodness=goodcorr;
		goodness2=goodcorr2;
		goodness3=goodcorr3;
		zigazaga=zigazagacorr;
		nzeros=nzeroscorr;
	}
	else {
		amplitud=ampliprev;
                T2=Tprev;
                goodness=goodprev;
                goodness2=goodprev2;
                goodness3=goodprev3;
		zigazaga=zigazagaprev;
		nzeros=nzerosprev;
	}
	fprintf(err, "Main Period=%lld Iters=%f\n", T2/1000000, (1.0*(t1-t0))/(1.0*T2/1000000));
	/*if(T2>999999 & T2!=1000000 & (!(goodness==0 & goodness2==0)|zigazaga==1) &
	(1.0*(t1-t0))/(1.0*T2/1000000)>3.5) {*/
	if (T2>999999 && ((goodness > 0.7 && goodness < 1.3 && goodness2 > 0.7 && goodness2< 10.3 &&
	goodness3 < 0.9) || zigazaga==1) && i<3 && cut==1 && T2/1000000<(t1-t0)/3.5) {
		for(count=0;count<i; count++) {fprintf(out, "   ");}
		fprintf(out, "Region from %ld to %ld ms\n", r0, r1);
		for(count=0;count<i; count++) {fprintf(out, "   ");} fprintf(out, "Stats:\n");
		for(count=0;count<i; count++) {fprintf(out, "   ");} fprintf(out, "Structure:\n");
		for(count=0;count<i+1; count++) {fprintf(out, "   ");}

        fprintf(out, "Iters=%f Main Period=%lld ms Likeliness=(%lf,%lf,%lf)\n", (1.0*(duration))/(1.0*T2/1000000), T2/1000000, goodness, goodness2, goodness3);
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
	else {
		/*for(count=0;count<i+1; count++) {printf("   ");} 
		printf("No period found\n");*/
		T2=0;
	}

	if(trace!=NULL) {
		sprintf(filename, "%s.%s.chop%d.prv", trace, option, num_chop);
                sprintf(filename2, "%s.%s.chop%d.prv", trace, option, num_chop);
                sprintf(filename4, "%s.%s.chop%d.prv", trace, option, num_chop);


/* DE FORMA PROVISIONAL UTILITZEM UN ALTRE RENAMING
		sprintf(filename, "%s.%s.%d.%d.%d", trace, option, n, i, j2);
        	sprintf(filename2, "%s.%s.%d.%d.%d", trace, option, n, i, j2);
		sprintf(filename4, "%s.%s.%d.%d.%d", trace, option, n, i, j2);
        	memcpy(filename+strlen(filename), ".levelnot.prv", strlen(".levelnot.prv")+1);
        	memcpy(filename2+strlen(filename2), ".sc.levelnot.prv", strlen(".sc.levelnot.prv")+1);
		memcpy(filename4+strlen(filename4), ".shiftednot.level.prv", strlen(".shiftednot.level.prv")+1);*/
	}

        j=0;
        T3=1;
        k=0;
	
	/*Identifying 2 periods */
	
	
	/* Crosscorr*/

	Generatesinus((t1-t0)*1000000, T2, 5000000, sin_txt, ((float)(duration))/((float)T2/1000000));
	
	Sampler(outin_txt, outin_samp_txt, (long long int)5000000);
	
	Crosscorrelation(outin_samp_txt, sin_txt, Crosscorrelation_txt);
	
	GS_FOPEN(gp, Crosscorrelation_txt, R);
	fscanf(gp, "%lld", &min2);
	GS_FCLOSE(gp);
	
	//j=(t1-t0)*1000000/T2;
	
	fprintf(err, "Min accordin to sinus=%lld\n", min+min2*5000000);
	
	
	/* We cut the original trace in order to provide a 2 periods trace. */

	if(signal3!=NULL) { 

	        Cutter_signal(signal3, signalt_txt, min+min2*5000000, min+min2*5000000+T2); 
        	max=Maximum(signalt_txt);
	}
	else {
		//max=min+min2*5000000;
		Cutter_signal(signal, signalt_txt, min+min2*5000000, min+min2*5000000+T2);
        	max=Maximum(signalt_txt);
	}

	
	//Cutter_signal(signal2, "signal2.cut.txt",  min+(j-1)*T2 + max, min+(j+1)*T2 + max);        
	//Take_regions("signal2.cut.txt", min/1000000+(j-1)*T2/1000000+max/1000000, shift);
        //fprintf(stderr, "shift %ld %ld\n", shift[0], shift[1]);

        /*if( shift[0]< shift[1]) {
                Cutter3(trace, "mer.prv", shift[0]-10, shift[1]+10);
                Compute_shift("mer.prv", "shift.txt");
        }*/
	
	/* tall sensible al tamany */
	if (trace != NULL && T2!=0)
	{

		printf("\nCalculating size of 1 iteration...");	fflush(stdout);
		min2=min2/2; 
		Cutter3(trace, filename,  min+min2*5000000+max, min+min2*5000000+max+T2, 0);

		GS_FOPEN(pFile, filename, R);
        	fseek (pFile, 0L, SEEK_END);
        	size=ftell(pFile);
       		GS_FCLOSE(pFile);
	
		Cutter3(trace, filename,  min+min2*5000000+max+2*T2, min+min2*5000000+max+3*T2, 0);
	
		GS_FOPEN(pFile2, filename, R);
        	fseek (pFile2, 0L, SEEK_END);
        	size2=ftell(pFile2);
       		GS_FCLOSE(pFile2);
		printf("Done!\n"); fflush(stdout); 	
		fprintf(err, "size1=%lld\n", size);
		fprintf(err, "size2=%lld\n", size2);


        	/* We look for the environment var in order to get max trace size */
        	if((env_var = getenv("SPECTRAL_TRACE_SIZE")) != NULL)
			spectral_trace_size = atoll(env_var);
		else
			spectral_trace_size = 100000000;

	
		if (size2 > size) {
			size=size2;
			max=max+2*T2;
		}
	
		if (size*2 > spectral_trace_size) {
			fprintf(err, "1111\n");
			n_iters=2;
		} else {
			fprintf(err, "2222\n");
			n_iters= spectral_trace_size/size;
		}
		
		if (min+min2*5000000+max+n_iters*T2 > t1*1000000) {
			if (n_iters > 2) {
			
				//n_iters =  (t1*1000000 - (min+min2*5000000+max))/T2;
			
			
				while (n_iters * T2 > (t1-t0) * 1000000 - max) {
			
					n_iters--;
			
				}
			
				n_iters--;
			
				min2 =  (t1-t0)*1000000 - max - (n_iters*T2);
			
				min2 = min2 / 5000000;
			
			
			
				Cutter_signal(signal3, signalt_txt, min+min2*5000000, min+min2*5000000+T2); 
        			max=Maximum(signalt_txt);
			
			
			
			} else {
				fprintf(err, "4444\n");
				max=max-2*T2;
			}
			
		}

	}
	else n_iters=MAX(requested_iters, 2);
	


	/* fi tall sensible al tamany */

	
	if (T2>999999 && ((goodness > 0.7 && goodness < 1.3 && goodness2 > 0.7 && goodness2< 10.3 &&
	goodness3 < 0.9) || zigazaga==1) && i<3 && cut==1 && T2/1000000<(t1-t0)/3.5) {
		//Cutter2(trace, filename,  min/1000000+(j-1)*T2/1000000, min/1000000+(j+1)*T2/1000000);
		if(trace!=NULL) {
			Cutter2(trace, filename,  min+min2*5000000+max,
			min+min2*5000000+max+n_iters*T2);
		}
		Cutter_signal(signal, signalout,  min+min2*5000000+max+T2,
		min+min2*5000000+max+n_iters*T2);
		//Paramedir(filename, "histograma");
        	//GS_FOPEN(pFile,filename, R);
        	//fseek (pFile, 0, SEEK_END);
       		//size=ftell(pFile);
       		//GS_FCLOSE(pFile);
       		//if(size>=100000000) {
		//	for(count=0;count<i+1; count++) {fprintf(out, "   ");} 
               	//	Sc(filename, filename2, 5000000);
		//	fprintf(out, "Trace for two good iters between %lld and %lld ns is in %s file\n", min+min2*5000000+max, min+min2*5000000+max+2*T2, filename2);
       		//}
      		//else {
			for(count=0;count<i+1; count++) {fprintf(out, "   ");} 
			fprintf(out, "Detected %d iters between %lld and %lld ns\n",
			n_iters, min+min2*5000000+max, min+min2*5000000+max+n_iters*T2);

			currentPeriod->best_ini = min+min2*5000000+max;
			currentPeriod->best_end = min+min2*5000000+max+n_iters*T2;

/* Make sure the recommended iters are inside the region of data */
#if 0
			if (currentPeriod->best_end > currentPeriod->end) { 
				currentPeriod->best_ini = currentPeriod->best_ini - (currentPeriod->best_end-currentPeriod->end);
				currentPeriod->best_end = currentPeriod->end;
				if (currentPeriod->best_ini < currentPeriod->ini) {
			    	  fprintf(err, "Not enough iterations detected.\n");
				}
			} 
#else
			while (currentPeriod->best_end > currentPeriod->end)
			{
				currentPeriod->best_ini -= currentPeriod->length * 1000000;
				currentPeriod->best_end -= currentPeriod->length * 1000000;
			}
			if (currentPeriod->best_ini < currentPeriod->ini) {
				currentPeriod->best_ini = currentPeriod->ini;
				fprintf(err, "Detected less iterations than requested!\n");
			}
#endif

			
       		//}
		*point1=min/1000000+min2*5+max/1000000;
		*point2=min/1000000+min2*5+max/1000000+n_iters*T2/1000000;
		//if(size>=50000000) {
		//	cut=1;
		//}
		//else {
		//	cut=0;
		//}
                /*if( shift[0]< shift[1]) {
                        Task_shifter("shift.txt", filename, filename4);
                        for(count=0;count<i+1; count++) {printf("   ");}
                        printf("Shifted trace: %s\n", filename4);
                }*/

	}
	return currentPeriod;
}

/* DEAD_CODE
void BarlettWindowing(struct signal_elem *sig, signalSize siglen)
{
    signalSize n;

    float Mid = (float)(siglen - 1) / (float)2;
    float K = ((float)2 / (float)(siglen - 1));
    float windowing = 0;

    for (n=0; n<siglen; n++)
    {
        windowing = K * ( Mid - fabsf( n - Mid ) );
        sig[n].value = sig[n].value * windowing;
    }
}
*/

static float windowingBarlett(signalSize idx, signalSize siglen)
{
    float Mid = (float)(siglen - 1) / (float)2;
    float K = ((float)2 / (float)(siglen - 1));
    float windowing = 0;

    windowing = K * ( Mid - fabsf( idx - Mid ) );
    return windowing;
}

void applyWindowing(struct signal_elem *sig, signalSize siglen, int window_type)
{
    signalSize n;

fprintf(stderr, "applyWindowing CALLED\n");

    for (n=0; n<siglen; n++)
    {
        float factor = 1;

        switch(window_type)
        {
            case W_BARLETT:
                factor = windowingBarlett(n, siglen);
                break;
            case W_10PCT:
                if ( n < siglen*0.1) factor = windowingBarlett(n, siglen);
                break;
            case W_NONE:
            default:
                factor = 1;
                break;
        }
        sig[n].value = sig[n].value * factor;
    }
}

#ifndef LIB_MODE
int main(int argc, char *argv[])
{
long int temps = time(NULL);
long int t0_flushing[200], t1_flushing[200], c, d, *periods, t0, t1;
long long int a, b, *totaltime, min, a2, b2;
long long int filt, size;
int *p, n_points;
int j, i, n, k, pfound, counter, trace_num, change, num_chop;
double f1, f2;
char filename[1024], systema[1024], **signals, **traces;
FILE *gp, *hp, *jp, *kp; 



if (argc<3) {
	printf("Usage: %s <first prv file> <second prv file> ... <MPIp2p or CPUDurBurst or IPC>\n", argv[0]);
	exit (0);
}

ini_rand();


/* Ini structs */
periods=(long int *)malloc(sizeof(long int) * (argc-2));
totaltime=(long long int *)malloc(sizeof(long long int) * (argc-2));
p=(int *)malloc(sizeof(int) * (argc-2));
signals = (char **)malloc(sizeof(char*) * (argc-2));
for(i=1; i<argc-1; i++) {
	signals[i]=(char *)malloc(sizeof(char) * 256);
}
traces = (char **)malloc(sizeof(char*) * (argc-2));
for(i=1; i<argc-1; i++) {
	traces[i]=(char *)malloc(sizeof(char) * 256);
}


change=0;

/*Extraction of the execution time and the number of threads*/

for(trace_num=1; trace_num < argc-1; trace_num++) {
	totaltime[trace_num]=Totaltime(argv[trace_num]);
	p[trace_num]=Nthreads(argv[trace_num]);
}


GS_ON();

GS_FOPEN(hp,"report.out", W);

GS_FOPEN(jp, "report.err", W);


/*loop of the tracefiles*/

for(trace_num=1; trace_num < argc-1; trace_num++) {


j=0;
i=0;
n=0;
num_chop = 0;

sprintf(signals[trace_num], "%s.1it.txt", argv[trace_num]);

fprintf(hp, "Trace: %s\n", argv[trace_num]);
fprintf(hp, "Metric: %s\n", argv[argc-1]);
fprintf(hp, "Totaltime=%lld ms\n", totaltime[trace_num]/1000000);
fprintf(hp, "Totalthreads=%d\n", p[trace_num]);

GS_FOPEN(kp, argv[trace_num], R);
 	fseek(kp, 0L, SEEK_END);
size=ftello64(kp);
	GS_FCLOSE(kp);
printf("Size=%lld\n", size);
fprintf(hp, "Size=%lld\n", size);


//filt=Filtering(argv[trace_num], "-bursts_histo");

/*generation of the filtered trace which contains flushing events*/

//Trace_filter(argv[trace_num], "out_.prv", 40000003);

/*we generate the flushing signal*/

printf("\nGenerating flushing signal..."); fflush(stdout);
GenerateEventSignal(argv[trace_num], "signal.txt", 40000003);
printf("Done!\n\n");
fprintf(hp, "\n");
GetBoundary("signal.txt", "tall.txt", totaltime[trace_num]);

/* we get the boundary of the flushing regions */

GS_FOPEN(gp,"tall.txt", R);

t0_flushing[i]=0;
t1_flushing[i]=totaltime[trace_num]/1000000;



while(fscanf(gp, "%lld %lld\n", &a, &b)==2) {
	a=a/1000000;
	b=b/1000000;
        
fprintf(stderr, "a %lld b %lld totaltime %lld\n", a, b, totaltime[trace_num]/1000000);

	if(a>0 & a<totaltime[trace_num]/1000000) {
		t1_flushing[i]=a;
		if(b<totaltime[trace_num]/1000000) { 
			i++;
			t0_flushing[i]=b;
			t1_flushing[i]=totaltime[trace_num]/1000000;
		}
	}
	if(a<=0 & b<totaltime[trace_num]/1000000) {
		t0_flushing[i]=b;
		t1_flushing[i]=totaltime[trace_num]/1000000;
	}
	if (a>totaltime[trace_num]/1000000) {
		t1_flushing[i]=totaltime[trace_num]/1000000;
	}
	
}
fprintf(stderr, "t0_flushing size=%d\n", i);
fprintf(stderr, "t1_flushing size=%d\n", i);

int kk=0;
for (kk=0; kk<=i; kk++)
{
  fprintf(stderr, "t0_f=%lld t1_f=%lld\n", t0_flushing[kk], t1_flushing[kk]);
}

GS_FCLOSE(gp);


/*we generate the signal we will study*/

sprintf(filename, "trace.%s.filtered.prv", argv[argc-1]);

counter=0;
pfound=0;



fprintf(hp, "Filtered trace: %s.%s.filtered.prv\n", argv[trace_num], argv[argc-1]);

while(counter < 2 && pfound==0) {

	if(strcmp(argv[argc-1], "BW")==0) {
		Trace_filter_IPC(argv[trace_num], filename);
		SignalBW(argv[trace_num], "signal.txt");
		Sampler_wavelet("signal.txt", "signal.samp.txt", 1024);
	}

	if(strcmp(argv[argc-1], "IPC")==0) {
		Trace_filter_IPC(argv[trace_num], filename);

		printf("Generating IPC Signal..."); fflush(stdout);
		SignalIPC(filename, "signal.txt");
		printf("Done!\n"); fflush(stdout);

                printf("Executing Sampler Wavelet..."); fflush(stdout);
		Sampler_wavelet("signal.txt", "signal.samp.txt", 4096);
                printf("Done!\n"); fflush(stdout);

	}
	
	if(strcmp(argv[argc-1],"MPIp2p")==0) {
		Trace_filter(argv[trace_num], filename, 50000001);

		printf("Generating MPIp2p Signal..."); fflush(stdout);
	        GenerateEventSignal(filename, "signal.txt", 50000001);
		printf("Done!\n"); fflush(stdout);

		printf("Executing Sampler Wavelet..."); fflush(stdout);
		Sampler_wavelet("signal.txt", "signal.samp.txt", 4096);
		printf("Done!\n"); fflush(stdout);
	}

	if(strcmp(argv[argc-1],"CPUBurst")==0) { 
		FilterRunning(argv[trace_num], filename,  totaltime[trace_num]/10000000);
		SignalRunning(filename, "signal.txt");
		Sampler_wavelet("signal.txt", "signal.samp.txt", 4096);	
	}

	if(strcmp(argv[argc-1],"CPUDurBurstlog")==0) {
        	FilterRunning(argv[trace_num], filename,  totaltime[trace_num]/10000000);
	        SignalDurRunning(filename, "signal.txt");
		SignalRunning(filename, "signal.log.txt");
		Sampler_wavelet("signal.log.txt", "signal.samp.txt", 4096);
	}

	if(strcmp(argv[argc-1],"CPUDurBurst")==0) {

		if (size<10000000000) {n_points=4096;}
		else {n_points=655360;}

	

        	FilterRunning(argv[trace_num], filename,  totaltime[trace_num]/100000);

		fprintf(hp, "Filtering=%lld ns\n", totaltime[trace_num]/100000);
		fprintf(hp, "N_points=%d \n", n_points);		

	        printf("Generating Running Signal..."); fflush(stdout);
                SignalRunning(filename, "signal.txt");
		printf("Done!\n"); fflush(stdout);

		printf("Executing Sampler Wavelet..."); fflush(stdout);
        	Sampler_wavelet("signal.txt", "signal.samp.txt", n_points);
		printf("Done!\n"); fflush(stdout); 

		printf("Generating DurRunning Signal..."); fflush(stdout);
		SignalDurRunning(filename, "signal.txt");
		printf("Done!\n"); fflush(stdout);
	}



	//Trace_filter(argv[trace_num], "out_2.prv", 50000002);
        //Generate_Signal("out_2.prv", "signal2.txt", 50000002);


	/*Global analysis using wavelets */
	
	Wavelet("signal.samp.txt", "PPP2");


	GS_FOPEN(gp,"PPP2", R);
	fprintf(hp, "Global Periodic Zones\n");
	n=0;
	j=0;
	pfound=0;
	
	/*we generate running burst signal anyway to obtain a right cut if the traces*/

	if(strcmp(argv[argc-1],"CPUDurBurst")!=0 && strcmp(argv[argc-1],"CPUBurst")!=0) {
                FilterRunning(argv[trace_num], "trace.prv",  totaltime[trace_num]/10000000);
                SignalDurRunning("trace.prv", "signal3.txt");	
	}
	else {	
		SignalDurRunning(filename, "signal3.txt");
	}

	/*selection of the peridical regions without flushing*/

	while(fscanf(gp, "%lf %lf\n", &f1, &f2)==2) {
		if(f1<f2) {
			c=totaltime[trace_num]/1000000*f1;
			d=totaltime[trace_num]/1000000*f2;
			a=0;
			b=0;
			a2=0;
			b2=0;
			fprintf(jp, "From %ld to %ld ms\n", c, d);
fprintf(stdout, "From %ld to %ld ms  num_flushes=%d\n", c, d, i);

			//printf("From %ld to %ld ms\n", c, d);
			min=0;
			k=0;
			for(j=0; j<=i; j++) {
				//printf("Flush: %ld %ld\n", t0_flushing[j], t1_flushing[j]);
				if(c<t0_flushing[j] & d>t1_flushing[j]) {
fprintf(stdout, "CASE 1\n");
					if(min<t1_flushing[j]- t0_flushing[j]) {
						a=t0_flushing[j];
						b=t1_flushing[j];
                                                a2=t0_flushing[j+3];
                                                b2=t1_flushing[j+3];
						min=b-a;
						k=j;
					}
				}
				if(t0_flushing[j]<=c &  d>t1_flushing[j]) {
fprintf(stdout, "CASE 2\n");
					if(min<t1_flushing[j]-c) {
                	                        a=c;
                        	                b=t1_flushing[j];
                                	        min=b-a;
						k=j;
                                	}
				}
				if(c<t0_flushing[j] & t0_flushing[j]<=d & t1_flushing[j]>=d) {
fprintf(stdout, "CASE 3\n");
					if(min<d - t0_flushing[j]) {
                	                        a=t0_flushing[j];
                        	                b=d;
                                	        min=b-a;
						k=j;
                                	}
				}	
				if(t0_flushing[j]<=c & t1_flushing[j]>=d) {
fprintf(stdout, "CASE 4 t0_flushing[%d]=%lld <= c=%lld && t1_flushing[%d]=%lld >= d=%lld? \nmin=%lld d=%lld c=%lld d-c=%lld\n", j, t0_flushing[j], c, j, t1_flushing[j], d, min, d, c, d-c);
					if(min<d-c) {
                	                        a=c;
                        	                b=d;
                                	        min=b-a;
						k=j;
                                	}
				}
			}
			fprintf(jp, "%lld %lld\n", a, b);
			//printf("%lld %lld\n", a, b);
			/*analysis of the regions*/
			//a=763;
			//b=257698;

			if(b!=0 & (d-c)>0.05*totaltime[trace_num]/1000000) {
				num_chop++;
				fprintf(hp, "\n");
				printf("\nExecuting analysis..."); fflush(stdout);
				Analysis("signal.txt", a, b, d-c, c, d, n, 0, k, argv[argc-1],
				argv[trace_num], 1, p[trace_num],
				"signal.txt", "signal3.txt", &pfound, &periods[trace_num], signals[trace_num],
				&t0, &t1, traces[trace_num], hp, jp, num_chop, ".", 0);
			}
			n++;
			if(change==1) {
				FilterRunning(argv[trace_num], filename, filt);
	        		SignalDurRunning(filename, "signal.txt");
				Cutter_signal("signal.txt", signals[trace_num], t0, t1);
			}
		}
	}

	GS_FCLOSE(gp);
	if(pfound==0 & strcmp(argv[argc-1], "CPUBurst")!=0) { //Problema que cal solucionar !!
		strcpy(argv[argc-1], "CPUBurst");
		counter++;
		fprintf(hp, "Metric: %s\n", argv[argc-1]);
		change=1;
	}
	else {
		counter=2;
	}
}

fprintf(hp, "\n\n");

sprintf(systema, "cp %s %s.%s.filtered.prv", filename, argv[trace_num], argv[argc-1]);

system(systema);

}



fprintf(hp, "Speedup Analysis\n");
for(trace_num=1; trace_num < argc-1; trace_num++) {
	fprintf(hp, "Trace %d : %f\n", trace_num, (1.0*periods[1])/(1.0*periods[trace_num]));
	 
}

GS_FCLOSE(hp);
GS_FCLOSE(jp);

GS_OFF(0);

//system("rm *.txt trace.*.filtered.prv trace.*.filtered.pcf PPP2 KK Tcorrect");

printf("Success!!!");

temps = time(NULL) - temps;
printf("%li Hours, %li Minutes, %li Seconds\n", temps/3600, (temps%3600)/60,(temps%3600)%60);

	return 0;
}

#endif

