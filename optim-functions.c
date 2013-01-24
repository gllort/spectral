#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <complex.h>
#include <fftw3.h>
#if defined (GS_VERSION)
	#include <GS_worker.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>
#include "optim.h"
#include "optim-macros.h"
#include "optim-functions.h"

char * Get_Binary_Dir (char *home_envvar, char *binary)
{
    char *envvalue;
    char path[MAXPATHLEN];
    struct stat buf;

    /* Check for the environment variable */
    envvalue = getenv(home_envvar);
    if (envvalue == NULL)
    {
        fprintf(stderr, "Error: %s environment variable is not defined!\n", home_envvar);
        exit(1);
    }

    /* Search for the binary in the directory pointed by the variable */
    snprintf(path, sizeof(path), "%s/%s", envvalue, binary);
    if (stat (path, &buf) != 0)
    {
        fprintf(stderr, "Error: Cannot find '%s' in '%s' (%s).\n", binary, envvalue, home_envvar);
        exit(1);
    }
    return envvalue;
}

void Sampler_wavelet(char * input, char * output, int number_of_points) {

  FILE *fileIn, *fileOut;
  long long int period, last_signal_time, time, delta;
  char buffer[512];
  double value, last_value;
  int calculated_periods = 0;


  /* open input file */
  if((fileIn = fopen(input, "r")) == NULL)
  {
        printf("\nSampler_wavelet: Can't open file %s !!!\n\n", input);
        exit(-1);
  }
                                                                                
  /* open output file */
  if((fileOut = fopen(output, "w")) == NULL)
  {
        printf("\nSampler_wavelet: Can't open file %s !!!\n\n", output);
        fclose(fileIn); exit(-1);
  }


  /* Obtaning de last time of the wavelet */
  fseek(fileIn, -150, SEEK_END);    
  while(fgets(buffer, 512, fileIn)!= NULL);
  sscanf(buffer, "%lld %*s\n", &last_signal_time);
  fseek(fileIn, 0 , SEEK_SET);

  /* Period = Total time / number of samples */
  period = last_signal_time / number_of_points;

  /* Calculating points */

  fscanf(fileIn, "%*lld %*lld %lf\n", &last_value);
  fscanf(fileIn, "%lld %lld %lf\n", &time, &delta, &value);

  

  int end_file = 0;
  while ( calculated_periods < number_of_points)
  {
	while(calculated_periods * period > time && !end_file)
	{
	   last_value = value; 
	   if(fscanf(fileIn, "%lld %lld %lf\n", &time, &delta, &value) == EOF) end_file = 1;
	}

	fprintf(fileOut, "%lf\n", last_value);
        calculated_periods++;

  }

  fclose(fileIn); fclose(fileOut);
}



void Wavelet(char * input, char * output) {
	char aux[200];
	
	sprintf(aux, "%s/bin/wavelet %s %s", SPECTRAL_HOME("bin/wavelet"), input, output);

	GS_SYSTEM(aux);
}




void GenerateEventSignal(char *input, char *output, long long int eventType) {

  FILE *fileIn, *fileOut;
  long long int type, value, signalValue;
  long long int last_time, current_time;
  char *buffer, events[4096], *token;


  /* open input file */
  if((fileIn = fopen(input, "r")) == NULL)
  {
	printf("\nGenerateEventSignal: Can't open file %s !!!\n\n", input);
	exit(-1);
  }

  /* open output file */
  if((fileOut = fopen(output, "w")) == NULL)
  {
        printf("\nGenerateEventSignal: Can't open file %s !!!\n\n", output);
        fclose(fileIn); exit(-1);
  }


  /* Alloc buffer for reading file */
  buffer = (char *)malloc(READ_BUFFER_SIZE);

  /* Parsing trace */
  last_time = 0; signalValue = 0;
  while(fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL)
  {
	/* Only events */
	if(buffer[0] == '2')
        {
	    sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%s\n", &current_time, events);
	    token = strtok(events, ":");
            do {
		/* Obtaining type and value */
		type = atoll(token);
		value = atoll(strtok(NULL, ":"));

		/* Generating Signal */
		if(type == eventType)
		{
		   if(current_time != last_time)
		   {
			fprintf(fileOut, "%lld %lld %lld\n", last_time, current_time - last_time, signalValue);
			last_time = current_time;
		   }

		   if(value != 0) signalValue++;
		   else signalValue--;

		   break;
		}
	
	    }
	    while((token = strtok(NULL, ":"))!=NULL); 

	}
  }

  fclose(fileIn); fclose(fileOut);
  free(buffer);
}




void Trace_filter(char * input, char * output, long int i) 
{
	char aux[200];
	
	sprintf(aux, "%s/trace_filter %s %s -show_events:%ld", FILTERS_HOME("trace_filter"), input, output, i);
	/*./trace_filter AMBER-PAR-MN-128tallada3.prv z.prv -show_events:50000001,:50000003*/
	
	GS_SYSTEM(aux);
}

void Trace_filter_IPC(char * input, char * output) 
{
	char aux[200];
	
	sprintf(aux, "%s/trace_filter %s %s -show_events:42000050,42000059,40000003", FILTERS_HOME("trace_filter"), input, output);
	
	GS_SYSTEM(aux);
}

void Cutter(char * input, char * output, int a, int b)
{
	char aux[200];	

	sprintf(aux, "%s/cutter %s %s -p 0 100 -tasks %d-%d", FILTERS_HOME("cutter"), input, output, a, b);
	
	GS_SYSTEM(aux);
}



void Cutter_signal(char * input, char * output, long long int t0, long long int t1)
{
   FILE *fileIn, *fileOut;
   long long int time, delta;
   float semanticValue;


  /* open input file */
  if((fileIn = fopen(input, "r")) == NULL)
  {
        printf("\nCuttingSignal: Can't open file %s !!!\n\n", input);
        exit(-1);
  }
                                                                                
  /* open output file */ 
  if((fileOut = fopen(output, "w")) == NULL)
  {
        printf("\nCuttingSignal: Can't open file %s !!!\n\n", output);
        fclose(fileIn); exit(-1);
  }


  /* Cutting signal */
  while(fscanf(fileIn, "%lld %lld %f\n", &time, &delta, &semanticValue)!=EOF)
	if((time + delta) >= t0)
	{
		fprintf(fileOut, "0 %lld %f\n", time + delta - t0, semanticValue);
		break;
	}


  while(fscanf(fileIn, "%lld %lld %f\n", &time, &delta, &semanticValue)!=EOF)
  {
	if((time + delta) <= t1)
		fprintf(fileOut, "%lld %lld %f\n", time - t0, delta, semanticValue);
	else
	{
		fprintf(fileOut, "%lld %lld %f\n", time - t0, t1 - time, semanticValue);
		break;
	}
  }


  fclose(fileIn); fclose(fileOut);
}
	




void Efficiency(char * input, char * output, int procs)
{

  FILE *fileIn, *fileOut;
  float value, eff = 0, eff_total = 0;
  long long int delta, delta_total = 0;


  /* open input file */
  if((fileIn = fopen(input, "r")) == NULL)
  {
        printf("\nEfficiency: Can't open file %s !!!\n\n", input);
        exit(-1);
  }
                                                                            
                                                                            
  /* open output file */
  if((fileOut = fopen(output, "w")) == NULL)
  {
        printf("\nEfficiency: Can't open file %s !!!\n\n", output);
        fclose(fileIn); exit(-1);
  }


  while(fscanf(fileIn, "%*lld %lld %f\n", &delta, &value)!=EOF)
  {
	if(value != 0)
	{
		eff += (float)delta * value;
		delta_total += delta;
		
	}

  }

  eff_total = eff / (float)procs;
  fprintf(fileOut, "%lf\n", eff_total / (float)delta_total);
  fclose(fileIn); fclose(fileOut);


}



void Crosscorrelation(char * input1, char * input2, char * output)
{

  fftw_complex *in=NULL, *out=NULL, *in2=NULL, *out2=NULL, *product=NULL, *product2=NULL;
  fftw_plan p, p2, p3, p4;
  int N=0, i=0, j=0, N1=0, N2=0;
  FILE *fileIn1=NULL, *fileIn2=NULL, *fileOut=NULL;
  double prova=0, *conj1=NULL, *conj2=NULL, max=0;

  /* open input file */
  if((fileIn1 = fopen(input1, "r")) == NULL)
  {
        printf("\nCrosscorrelation: Can't open file %s !!!\n\n", input1);  exit(-1);
  }


  /* open input file */
  if((fileIn2 = fopen(input2, "r")) == NULL)
  {
        printf("\nCrosscorrelation: Can't open file %s !!!\n\n", input2); 
	fclose(fileIn1); exit(-1);
  }


  /* open output file */
  if((fileOut = fopen(output, "w")) == NULL)
  {
        printf("\nCrosscorrelation: Can't open file %s !!!\n\n", output);
        fclose(fileIn1); fclose(fileIn2); exit(-1);
  }


  i=0;
  while(fscanf(fileIn1, "%lf ", &prova)==1) i++; 
  N1 = i;
  i=0;
  while(fscanf(fileIn2, "%lf ", &prova)==1) i++; 
  N2 = i;

  N = MAX(N1, N2);
//fprintf(stderr, "N1=%d N2=%d N=%d\n", N1, N2, N);

//  N=i;
  //printf("Cross: %d\n", i);
  fseek(fileIn1, 0L, SEEK_SET);
  fseek(fileIn2, 0L, SEEK_SET);
  in=fftw_malloc(sizeof(fftw_complex)*N);
  out=fftw_malloc(sizeof(fftw_complex)*N);
  in2=fftw_malloc(sizeof(fftw_complex)*N);
  out2=fftw_malloc(sizeof(fftw_complex)*N);
  product=fftw_malloc(sizeof(fftw_complex)*N);
  conj1=(double *)malloc(sizeof(double)*N);
  conj2=(double *)malloc(sizeof(double)*N);

  bzero(in, sizeof(fftw_complex)*N);
  bzero(out, sizeof(fftw_complex)*N);
  bzero(in2, sizeof(fftw_complex)*N);
  bzero(out2, sizeof(fftw_complex)*N);
  bzero(conj1, sizeof(double)*N);
  bzero(conj2, sizeof(double)*N);

  p=fftw_plan_dft_1d(N, in, out, -1, FFTW_ESTIMATE);
  p2=fftw_plan_dft_1d(N, in2, out2, -1, FFTW_ESTIMATE);

#if 0
  for(i=0; i<N; i++) {
       fscanf(fileIn1, "%lf ", &in[i]);
       fscanf(fileIn2, "%lf ", &in2[i]);
  }
#else
  for(i=0; i<N1; i++) {
       fscanf(fileIn1, "%lf ", &in[i]);
//fprintf(stderr, "in1[%d]=%lf\n", i, cabs(in[i]));
  }
  for(i=0; i<N2; i++) {
       fscanf(fileIn2, "%lf ", &in2[i]);
//fprintf(stderr, "in2[%d]=%lf\n", i, cabs(in2[i]));
  }
#endif
                                                                                
                                                                                
  fftw_execute(p); fftw_execute(p2);

                                                                                
  for(i=0; i<N; i++)  {
          product[i]=out[i]*conj(out2[i]);
//          product2[i]=out2[i]*conj(out[i]);
  }                                                          

                                                                                
  p3=fftw_plan_dft_1d(N, product, out, 1, FFTW_ESTIMATE);
  fftw_execute(p3);
//  p4=fftw_plan_dft_1d(N, product2, out2, 1, FFTW_ESTIMATE);
//  fftw_execute(p4);

/*
  for(i=0; i<N; i++)  {
     fprintf(stderr, "product_out1[%d]=%lf product_out2[%d]=%lf (equal? %d)\n",
        i, cabs(out[i]), i, cabs(out2[i]), ( cabs(out[i]) == cabs(out2[i]) ));
  }
*/                                                                                
  max = 0; j = 0;
                                                                                
  //ULL!! N/p on p nombre d'iteracions, en aquest cas 2
  for(i=0; i<N; i++) {
        conj1[i]=(double)(pow(cabs(out[i]),2)/pow(N,3));
                                                                                
        if(conj1[i]>max) {
                 max=conj1[i];
                 j=i;
        }
  }
                                                                                
  fprintf(fileOut, "%d %lf\n", j, max);
                                                                                
  for(i=0; i<N; i++) 
         fprintf(fileOut, "%d %lf\n", i, conj1[i]);

                       
  fclose(fileIn1); fclose(fileIn2); fclose(fileOut);
  free(in); free(out); free(in2); free(out2); free(product); free(conj1); free(conj2);

}
	
void Cutter2(char * input, char * output, long long int a, long long int b)
{
	char aux[2000];
	
	/*if(a!=0) {
		sprintf(aux, "./cutter %s %s -t %ld%s %ld%s -original_time -not_break_states -remove_last_state", input, output, a, "000000", b, "000000");
	}
	else {
		sprintf(aux, "./cutter %s %s -t %ld %ld%s -original_time -not_break_states -remove_last_state", input, output, a, b, "000000");
	}*/

	 sprintf(aux, "%s/cutter %s %s -t %lld %lld -original_time -not_break_states -remove_last_state", FILTERS_HOME("cutter"), input, output, a, b);
	
	GS_SYSTEM(aux);
}

void Cutter3(char * input, char * output, long long int a, long long int b, int show_messages)
{
        char aux[200];

        /*if(a!=0) {
                sprintf(aux, "./cutter %s %s -t %ld%s %ld%s -remove_last_state", input, output, a, "000000", b, "000000");
        }
        else {
                sprintf(aux, "./cutter %s %s -t %ld %ld%s -remove_last_state", input, output, a, b, "000000");        }*/
        if(show_messages)
	   sprintf(aux, "%s/cutter %s %s -t %lld %lld", FILTERS_HOME("cutter"), input, output, a, b);
	else
           sprintf(aux, "%s/cutter %s %s -t %lld %lld > /dev/null", FILTERS_HOME("cutter"), input, output, a, b);

        GS_SYSTEM(aux);
}


void SenyalD(char * input, long long int k, char * output) 
{
	char aux[200];

	
	sprintf(aux, "%s/bin/SenyalD %s %lld %s", SPECTRAL_HOME("bin/SenyalD"), input, k, output);

	GS_SYSTEM(aux);
}

void SenyalE(char * input, long long int k, char * output)
{
	char aux[200];
	
	
	sprintf(aux, "%s/bin/SenyalE %s %lld %s", SPECTRAL_HOME("bin/SenyalE"), input, k, output);
	
	GS_SYSTEM(aux);
	
}

void Compute_shift(char * input, char * output)
{
	char aux[200];

	sprintf(aux, "%s/bin/compute_shift.pl %s > %s", SPECTRAL_HOME("bin/compute_shift.pl"), input, output);

	GS_SYSTEM(aux);

}

void Task_shifter(char * time, char * input, char * output)
{
	char aux[200];

	sprintf(aux, "%s/task_shifter %s %s %s", FILTERS_HOME("task_shifter"), time, input, output);

	GS_SYSTEM(aux);
}


	
void Trace_filter2(char * input, char * output)
{
        char aux[200];

        sprintf(aux, "%s/trace_filter %s %s -show_events:50000001-50000003", FILTERS_HOME("trace_filter"), input, output);
        /*./trace_filter AMBER-PAR-MN-128tallada3.prv z.prv -show_events:50000001,:50000003*/

        GS_SYSTEM(aux);
}

void Trace_filter3(char * input, char * output)
{
	char aux[200];

	sprintf(aux, "%s/trace_filter %s %s -show_events:50000002", FILTERS_HOME("trace_filter"), input, output);

	GS_SYSTEM(aux);
}



void Sampler(char * input, char * output, long long int Freq)
{


        int N, M, i, j;
        FILE *fp, *gp;
        double prova, *conj, *conj2;
        long long int a, b;
        long long int freq;
        long long int *t, *delta;
        float *semanticvalue, *semanticvalue2;


        fp=fopen(input, "r");
        gp=fopen(output, "w");
        freq=Freq;
        i=0;
        while(fscanf(fp, "%lld %lld %lf ", &a, &b, &prova)==3) {
                i++;
        }
        N=i;
        //printf("FFFFF=%d\n", N);
        fseek(fp, 0L, SEEK_SET);
        t = (long long int *)malloc(sizeof(long long int)*N);
        delta = (long long int *)malloc(sizeof(long long int)*N);
        semanticvalue = (float *)malloc(sizeof(float)*N);
        j=0;
        while (fscanf(fp, "%lld%lld%f", &t[j], &delta[j], &semanticvalue[j])==3) {
                j++;
        }
        M=(t[N-1]+delta[N-1])/freq+1;
        semanticvalue2 = (float *)malloc(sizeof(float)*M);
        i=0;
        j=0;
        while(i<M) {
                while(i*freq > t[j]+delta[j] & j<N) {
                        j++;
                }
                semanticvalue2[i]=semanticvalue[j];
                /*printf("%d %d %f\n", M, i, semanticvalue2[i]);
                fprintf(gp, "%d %d %f\n", M, i, semanticvalue2[i]);*/
                fprintf(gp, "%f\n", semanticvalue2[i]);
                i++;
        }
        fclose(fp);
        fclose(gp);



/*
  int i;
  FILE *fileIn, *fileOut;
  long long int t, delta;
  float semanticvalue;


  if((fileIn = fopen(input, "r")) == NULL)
  {
        printf("\nSampler: Can't open file %s !!!\n\n", input);  exit(-1);
  }


  if((fileOut = fopen(output, "w")) == NULL)
  {
        printf("\nSampler: Can't open file %s !!!\n\n", output);
        fclose(fileIn); exit(-1);
  }

  
  i = 0;
  while(fscanf(fileIn, "%lld %lld %lf ", &t, &delta, &semanticvalue)!=EOF)
  {
	while(i*Freq <= t) 
	{
	   fprintf(fileOut, "%f\n", semanticvalue);
	   i++;
	}

  }

  fclose(fileIn); fclose(fileOut);
*/
}


void Sampler2(char * input, char * output)
{
  FILE *fileIn, *fileOut;
  float value;

  /* open input file */
  if((fileIn = fopen(input, "r")) == NULL)
  {
        printf("\nSampler2: Can't open file %s !!!\n\n", input);  exit(-1);
  }
                                                                                
  /* open output file */
  if((fileOut = fopen(output, "w")) == NULL)
  {
        printf("\nSampler2: Can't open file %s !!!\n\n", output);
        fclose(fileIn); exit(-1);
  }

  while(fscanf(fileIn, "%*lld %*lld %f\n", &value) != EOF)
 	fprintf(fileOut, "%f\n", value);

  fclose(fileIn); fclose(fileOut); 
}



void Fft(char * input, char * output, int Freq)
{
	char aux[200];

	/*sprintf(aux, "./fft %s %s 1000", input, output);*/
	sprintf(aux, "%s/bin/fft %s %s %d", SPECTRAL_HOME("bin/fft"), input, output, Freq);

	GS_SYSTEM(aux);
}


void Fftprev(char * input, char * output, int Freq)
{
	char aux[200];

	/*sprintf(aux, "./fft %s %s 1000", input, output);*/
	sprintf(aux, "%s/bin/fftprev %s %s %d", SPECTRAL_HOME("bin/fftprev"), input, output, Freq);

	GS_SYSTEM(aux);
}

void Sc(char * input, char * output, int Freq) 
{
	char aux[200];
	
	sprintf(aux, "%s/sc %s %s %d 1 -global_counters -only_counters", FILTERS_HOME("sc"), input, output, Freq);

	GS_SYSTEM(aux);
}


/* Function for use in qsort */
int qsort_cmp(const void *a, const void *b)
{
  long long int temp = ((struct burst_info *)a)->time - ((struct burst_info *)b)->time;
                                                                                
  if(temp < 0) return -1;
  if(temp == 0) return 0;
  if(temp > 0) return 1;
                                                                                
}


void SignalBW(char * input, char * output)
{

  FILE *fileIn, *fileOut;
  long long int i, time_1, time_2, last_time, size, num_comms = 0;
  char *buffer;
  struct burst_info *comm_times;

                                                                                
                                                                                
  /* open input file */
  if((fileIn = fopen(input, "r")) == NULL)
  {
        printf("\nGenerateSignalBw: Can't open file %s !!!\n\n", input);  exit(-1);
  }
                                                                                
  /* open output file */
  if((fileOut = fopen(output, "w")) == NULL)
  {
        printf("\nGenerateSignalBw: Can't open file %s !!!\n\n", output);
        fclose(fileIn); exit(-1);
  }


  /* Allocating memory for buffers */
  buffer = (char *)malloc(READ_BUFFER_SIZE);
  comm_times = (struct burst_info *)malloc( 20000000 * 2 * sizeof(struct burst_info));
                                                                                
                                                                                
  /* Parsing trace */
  while(fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL){
     if(buffer[0] == '3')
     {
        sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%*lld:%lld:%*d:%*d:%*d:%*d:%*lld:%lld:%lld\n", &time_1, &time_2, &size);
        
           comm_times[num_comms].time = time_1;
	   comm_times[num_comms + 1].time = time_2;
           if(time_1 != time_2)
	   {
              comm_times[num_comms].value = size/(time_2 - time_1);
	      comm_times[num_comms + 1].value = -(size/(time_2 - time_1));
	   }
	   else 
	   {
	      comm_times[num_comms].value = 0;
              comm_times[num_comms + 1].value = 0;
	   }
           num_comms += 2;

     }
  }



 /* Generating signal */
                                                                                
  // QSORT
  qsort(comm_times, num_comms, sizeof(struct burst_info), qsort_cmp);
                                                                                
  /* Dumping signal to disk */
  long long int acumm_value;

  last_time = 0; acumm_value = 0;
  if(comm_times[0].time - last_time > 0)
        fprintf(fileOut, "0 %lld 0\n", comm_times[0].time);
                                                                                
  for(i = 0; i < num_comms - 2; i++) {
        comm_times[i].value = comm_times[i].value + acumm_value;
        if(comm_times[i+1].time - comm_times[i].time > 0)
        {
           fprintf(fileOut, "%lld %lld %lld\n", comm_times[i].time, comm_times[i+1].time - comm_times[i].time, comm_times[i].value);
        }
  }
                                                                                
  fclose(fileIn); fclose(fileOut);
  free(buffer); free(comm_times);

}



void SignalRunning(char * input, char * output)
{
   
   FILE *fileIn, *fileOut;


   struct burst_info *burst_times;
  
   char *buffer;
   unsigned long long num_times = 0, i;
   long long int time_1, time_2;
   int state;



  /* open input file */
  if((fileIn = fopen(input, "r")) == NULL)
  {
        printf("\nGenerateRunningSignal: Can't open file %s !!!\n\n", input);
        exit(-1);
  }
                                                                                
  /* open output file */
  if((fileOut = fopen(output, "w")) == NULL)
  {
        printf("\nGenerateRunningSignal: Can't open file %s !!!\n\n", output);
        fclose(fileIn); exit(-1);
  }


  /* Allocating memory for buffers */
  buffer = (char *)malloc(READ_BUFFER_SIZE);
  burst_times = (struct burst_info *)malloc( 20000000 * 2 * sizeof(struct burst_info));

  

  /* Parsing trace */
  while(fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL){ 
     if(buffer[0] == '1')
     {
	sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%lld:%d\n", &time_1, &time_2, &state); 
	if(state == 1)
	{
	   burst_times[num_times].time = time_1;
	   burst_times[num_times].value = 1;
	   num_times++;
	   burst_times[num_times].time = time_2;
	   burst_times[num_times].value = -1;
	   num_times++;
	}
     }
  }


  /* Generating signal */

  // QSORT
  qsort(burst_times, num_times, sizeof(struct burst_info), qsort_cmp);

  /* Dumping signal to disk */
  if(burst_times[1].time - burst_times[0].time > 0)
	fprintf(fileOut, "%lld %lld %lld\n", burst_times[0].time, burst_times[1].time - burst_times[0].time, burst_times[0].value);

  for(i = 1; i < num_times - 2; i++) {
	burst_times[i].value = burst_times[i].value + burst_times[i-1].value;
	if(burst_times[i+1].time - burst_times[i].time > 0)
	{
	   fprintf(fileOut, "%lld %lld %lld\n", burst_times[i].time, burst_times[i+1].time - burst_times[i].time, burst_times[i].value);
	}
  }

  fclose(fileIn); fclose(fileOut);
  free(buffer); free(burst_times); 

}



void SignalDurRunning(char * input, char * output)
{

   FILE *fileIn, *fileOut;
                                                                                
                                                                                
   struct burst_info *burst_times;
                                                                                
   char *buffer;
   unsigned long long num_times = 0, i;
   long long int time_1, time_2;
   int state;


  /* open input file */
  if((fileIn = fopen(input, "r")) == NULL)
  {
        printf("\nGenerateDurRunningSignal: Can't open file %s !!!\n\n", input);
        exit(-1);
  }
                                                                                
  /* open output file */
  if((fileOut = fopen(output, "w")) == NULL)
  {
        printf("\nGenerateDurRunningSignal: Can't open file %s !!!\n\n", output);
        fclose(fileIn); exit(-1);
  }
                                                                                
                                                                                
  /* Allocating memory for buffers */
  buffer = (char *)malloc(READ_BUFFER_SIZE);
  burst_times = (struct burst_info *)malloc( 20000000 * 2 * sizeof(struct burst_info));
                                                                                


  /* Parsing trace */
  while(fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL){
     if(buffer[0] == '1')
     {
        sscanf(buffer, "%*d:%*d:%*d:%*d:%*d:%lld:%lld:%d\n", &time_1, &time_2, &state);
        if(state == 1)
        {
           burst_times[num_times].time = time_1;
           burst_times[num_times].value = time_2 - time_1;
           num_times++;
           burst_times[num_times].time = time_2;
           burst_times[num_times].value = -(time_2 - time_1);
           num_times++;
        }
     }
  }


  /* Generating signal */
                                                                                
  // QSORT
  qsort(burst_times, num_times, sizeof(struct burst_info), qsort_cmp);
                                                                                
  /* Dumping signal to disk */
  if(burst_times[1].time - burst_times[0].time > 0)
        fprintf(fileOut, "%lld %lld %lf\n", burst_times[0].time, burst_times[1].time - burst_times[0].time, (1.0 * burst_times[0].value)/1000000.0);
                                                                                
  for(i = 1; i < num_times - 2; i++) {
        burst_times[i].value = burst_times[i].value + burst_times[i-1].value;
        if(burst_times[i+1].time - burst_times[i].time > 0)
        {
           fprintf(fileOut, "%lld %lld %lf\n", burst_times[i].time, burst_times[i+1].time - burst_times[i].time, (1.0 * burst_times[i].value)/1000000.0);
        }
  }
                                                                                
  fclose(fileIn); fclose(fileOut);
  free(buffer); free(burst_times);

}


void SignalDurRunninglog(char * input, char * output)
{
        char aux[200];

        sprintf(aux, "%s/bin/GenerateSignalDurRunninglog %s %s", SPECTRAL_HOME("bin/GenerateSignalDurRunninglog"), input, output);

        GS_SYSTEM(aux);

}


void SignalIPC(char * input, char * output)
{

  FILE *fileIn, *fileOut;
  struct IPC_info *ipc_signal;
  long long int i, type, value, instructions, cycles, current_time, num_ipcs;
  char *buffer, events[4096], *token;
  int counter_found, cpu;
  double last_IPC[10000];


  /* open input file */
  if((fileIn = fopen(input, "r")) == NULL)
  {
        printf("\nGenerateSignalIPC: Can't open file %s !!!\n\n", input);        exit(-1);
  }
                                                                                
  /* open output file */
  if((fileOut = fopen(output, "w")) == NULL)
  {
        printf("\nGenerateSignalIPC: Can't open file %s !!!\n\n", output);
        fclose(fileIn); exit(-1);
  }


  for(i = 0; i < 10000; i++) last_IPC[i] = 0.0;

  /* Alloc buffers */
  buffer = (char *)malloc(READ_BUFFER_SIZE);
  ipc_signal = (struct IPC_info *)malloc(20000000 * sizeof(struct IPC_info));


  /* Parsing trace */
  num_ipcs = 0;
  while(fgets(buffer, READ_BUFFER_SIZE, fileIn) != NULL)
  {
        /* Only events */
        if(buffer[0] == '2')
        {
	    counter_found = 0;
            sscanf(buffer, "%*d:%d:%*d:%*d:%*d:%lld:%s\n", &cpu, &current_time, events);
            token = strtok(events, ":");
            do {
                /* Obtaining type and value */
                type = atoll(token);
                value = atoll(strtok(NULL, ":"));
                                                                                
                /* Generating Signal */
                if(type == HW_COUNTER_INSTR)
                {
		   counter_found = 1;
		   instructions = value;
		}

		if(type == HW_COUNTER_CYC)
		{
		   counter_found = 1;
		   cycles = value;
		}
            }
            while((token = strtok(NULL, ":"))!=NULL);
            
	    if(counter_found)
	    {
		counter_found = 0;
		ipc_signal[num_ipcs].time = current_time;
		if(cycles != 0)
		   ipc_signal[num_ipcs].value = ((double)instructions/(double)cycles) - last_IPC[cpu];
		else
		   ipc_signal[num_ipcs].value = 0.0;

		last_IPC[cpu] = ipc_signal[num_ipcs].value;
		num_ipcs++;
	    }
	     
        }
  }
                                                                                

  /* Generating signal */
  double acumm_ipc = 0;
  for(i = 0; i < num_ipcs - 1; i++)
  {
	fprintf(fileOut, "%lld %lld %lf\n", ipc_signal[i].time, ipc_signal[i+1].time - ipc_signal[i].time, acumm_ipc);
	acumm_ipc += ipc_signal[i].value;
  } 

  fclose(fileIn); fclose(fileOut);
  free(buffer);

}



void FilterRunning(char * input, char * output, long long int t)
{
	char aux[200];

	sprintf(aux, "%s/trace_filter %s %s -show_events:40000003 -show_states_min_time:%lld", FILTERS_HOME("trace_filter"), input, output, t);
	//sprintf(aux, "./trace_filter %s %s -show_events:40000003 -show_states:%ld000000", input, output, t);
	/*sprintf(aux, "./trace_filter %s %s -show_states:100000000", input, output);10000000 era l'habitual*/

	GS_SYSTEM(aux); 
}

void Correction(char * Signal, long long int delta, char * Signal2)
{
	long long int t, d;
	float f;
	FILE *gp, *fp;
        char buffer[2048];


	gp = fopen(Signal, "r");
	fp = fopen(Signal2, "w");
/*
        while(fgets(buffer, sizeof(buffer), gp) != NULL)
	   fputs(buffer, fp);

        fclose(gp);

        sscanf(buffer, "%lld %lld %f\n", &t, &d, &f);
*/

// Codi de Marc 
	while(fscanf(gp, "%lld %lld %f", &t, &d, &f)==3) {
		fprintf(fp, "%lld %lld %f\n", t, d, f);
	}
	fclose(gp);


	if((t+d)/1000000 < delta) {
        	fprintf(fp, "%lld %lld 0\n",  t+d, delta*1000000-t-d);
	}
       	fclose(fp);

}


void Stats(char * input, char * output, char * option) {

	char aux[200];

	sprintf(aux, "%s/stats %s -only_dat_file -o %s %s", FILTERS_HOME("stats"), input, output, option);
	GS_SYSTEM(aux);
}




void GetTime(char * Freq, char * Signal, int mode, char * Tcorrect, char * output)
{
FILE *fp, *gp;
double p, f, limit, p1, p2, f1, f2, maxp, maxf, maxp2, maxf2, minp, minf, pant, fant, *max_, *max2;
int control, semanticval;
long long int t, delta, i, N, j;
int correct, zigazaga;

control=0;
t=0;
i=0;
maxp=0;
maxp2=0;
minp=10000000000;
correct=1;

//printf("%s\n", Freq);
//printf("%s\n", Signal);
//printf("%s\n", Tcorrect);

fp = fopen(Freq, "r");
gp = fopen(Signal, "r");

while(fscanf(fp, "%lf %lf", &f, &p)==2) {
        i++;
}
N=i;

max_ = (double *)malloc(sizeof(double) * N);

i=0;
fseek(fp, 0L, SEEK_SET);
fscanf(fp, "%lf %lf", &f2, &p2);
fscanf(fp, "%lf %lf", &fant, &pant);
while((pant==p2) && (i<N/2+1)) {
        fscanf(fp, "%lf %lf", &fant, &pant);
//	printf("hola %lf %lf\n", fant, pant);
        i++;
}
p1=pant;
f1=fant;

//printf("f2=%lf p2=%lf f1=%lf p1=%lf\n", f2, p2, f1, p1);


/* Provar d'inicialitzar les variables?*/

//printf("i=%lld j=%lld N/2+1=%lld\n", i, j, N/2+1);

max2=max_;

for(j=i; j<N/2+1; j++) {
	fscanf(fp, "%lf %lf", &f, &p);

//	printf("j=%lld f2=%lf p2=%lf f1=%lf p1=%lf f=%lf p=%lf\n", j, f2, p2, f1, p1, f, p);
	if(p1!=p) {
        if(p2<p1 & p1>p) {
//                printf("%lf %lf %lf\n", f1, p1, minp);
		*max_=f1;
		max_++;
                if(p1>maxp) {
                        maxp=p1;
                        maxf=f1;
                 }
                if(p1>maxp2 & p1<maxp) {
                        maxp2=p1;
                        maxf2=f1;
                }
                if(p1<minp) {
                        minp=p1;
                        minf=f1;
                }
        }
	
        p2=p1;
        f2=f1;
        p1=p;
        f1=f;
	/*fscanf(fp, "%lf %lf", &fant, &pant);
        while(pant==p & i<N/2+1) {
                fscanf(fp, "%lf %lf", &fant, &pant);
                i++;
        }*/
	}

}

*max_=-1.0;
max_=max2;

//printf("Period=%lf ms\n", maxf/1000000);
if(mode==0) {
        for(limit=0; limit<maxf+1; limit++) {
                fscanf(gp, "%lld %lld %d", &t, &delta, &semanticval);
        }
}
else {
        t=maxf;
}

//printf("maxp2=%lf, maxp=%lf, maxf2=%lf, maxf=%lf\n", maxp2, maxp, maxf2, maxf);
//printf("maxp2/maxp=%lf, min(maxf2, maxf) / max(maxf2, maxf)=%lf\n", maxp2/maxp,  MIN(maxf2, maxf) / MAX(maxf2, maxf));

//printf("minp=%lf, maxp=%lf, minp/maxp=%lf\n", minp, maxp, minp/maxp);
if((maxp2/maxp < 0.9) || (MIN(maxf2, maxf) / MAX(maxf2, maxf) < 0.9)) {
        //printf("Warning!!!! -----> Two similar periods\n");
        correct=0;
}

if(minp/maxp > 0.99) {
        //printf("Warning!!!! -----> There are not significant periods\n");
        correct=0;
}

fclose(fp);
fclose(gp);

N=2;

zigazaga=1;

fp = fopen(Freq, "r");
gp = fopen(output, "a+");

while((fscanf(fp, "%lf %lf", &f, &p)==2) && (N<5) && (zigazaga==1)) {
	fprintf(gp, "%lf %lf\n", 1.0*N*t, f);
	
	if(/*0.95*f < N*t & N*t < 1.05*f*/ f==N*t) {
		fprintf(gp, "%lf\n", *max_);
                while((*max_ < f) && (*max_!=-1)) {
			max_++;
                }
		//printf("%lf\n", *max_);
		if((0.95*f < *max_) && (*max_ < 1.05*f)) {
			fprintf(gp, "%lf %lf %lf %lf\n", maxp, maxf, p, f);
			N++;
		}
                else {
                        zigazaga=0;
                }
	}
	/*if(N*t==f) {
		if(0.9*maxp < p) {
			printf("%lf %lf %lf %lf\n", maxp, maxf, p, f);
			N++;
		}
		else {
			zigazaga=0;
		}
	}*/
}
if(N<5) {
	zigazaga=0;
}


/*if (zigazaga==1) {
	correct=1;
}*/

fclose(fp);
fclose(gp);

if( (maxf2/maxf<=2.05 && maxf2/maxf>=1.95) || (maxf2/maxf<=3.05 && maxf2/maxf>=2.95)) {zigazaga=1;}
	

fp=fopen(Tcorrect, "w");
fprintf(fp, "%lld %d %lf %lf %lf %d %lld\n", t, correct, maxp2/maxp, maxf2/maxf, minp/maxp, zigazaga, N);
fclose(fp);


}

