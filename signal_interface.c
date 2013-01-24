

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "signal_interface.h"
#include "optim-functions.h"
#include "optim-macros.h"
#include "optim.h"
#include "math.h"

signalSize copySignal(struct signal_elem *signal_in, signalSize sigsize_in, struct signal_elem **signal_out)
{
	int i = 0;
	signalSize sigsize_res = sigsize_in;
	struct signal_elem *signal_res = NULL;

	signal_res = (struct signal_elem *)malloc(sigsize_res * sizeof(struct signal_elem));
	if (signal_res != NULL)
	{
		for (i=0; i<sigsize_res; i++)
		{
			signal_res[i].time  = signal_in[i].time;
			signal_res[i].delta = signal_in[i].delta;
			signal_res[i].value = signal_in[i].value;
		}
	}
	else
	{
		sigsize_res = 0;
	}

	*signal_out = signal_res;
	return sigsize_res;
}

void serializeSignal(struct signal_elem *signal, signalSize sigsize, long long int **times_out, long long int **deltas_out, long long int **values_out)
{
	int i = 0;
	long long int *times=NULL, *deltas=NULL, *values=NULL;

	times  = (long long int *)malloc(sigsize * sizeof(long long int));
	deltas = (long long int *)malloc(sigsize * sizeof(long long int));
	values = (long long int *)malloc(sigsize * sizeof(long long int));

	for (i=0; i<sigsize; i++)
	{
		times[i]  = signal[i].time;
		deltas[i] = signal[i].delta;
		values[i] = signal[i].value;
	}
	
	*times_out  = times;
	*deltas_out = deltas;
	*values_out = values;
}

signalSize generateSignal(long long int *times, long long int *deltas, long long int *values, signalSize input_size, struct signal_elem **signalOut)
{
	int i = 0, count = 0;
	struct signal_elem *signal = NULL;

	if (input_size > 0)
	{
		/* Allocating memory for signal */
		if((signal = (struct signal_elem *)malloc(sizeof(struct signal_elem) * ((input_size*2)+1))) != NULL)
		{
#if 0
			/* Make sure the signal starts at zero, otherwise the analysis detects less and less iters as time moves forward */
			if (times[0] > 0)
			{
				signal[0].time  = 0;
				signal[0].delta = times[0];
				signal[0].value = 0;
				count ++;
			}
#endif

			/* Generate signal values */
			for(i = 0; i < input_size; i++)
			{
				signal[count].time = times[i];
				signal[count].delta = deltas[i];
				signal[count].value = values[i];
				count ++;

#if 0 /* XXX Using the new addSorting we can remove this... */

				/* Fill the gaps with value 0 */
				if ((i+1 < input_size) && (times[i] + deltas[i] < times[i+1]))
				{
					signal[count].time = times[i] + deltas[i];
					signal[count].delta = times[i+1] - signal[count].time;
					signal[count].value = 0;
					count ++;
				}
#endif
			}
			signal = (struct signal_elem *)realloc(signal, sizeof(struct signal_elem) * count);
		}
	}
	*signalOut = signal;
	return count;
}

int compressSignal(struct signal_elem **signalOut, signalSize signal_size, long long int min_delta)
{
	int i, j, k;
	long long int delta;
	struct signal_elem *signal = *signalOut;

	k = 0;
	for (i=0; i<signal_size; i++)
	{
		if (signal[i].delta == 0) continue;

		j = i+1;
		delta = signal[i].delta;
		while ((j < signal_size) && ((signal[j].value == signal[i].value) || (signal[j].delta == 0) || (signal[j].delta < min_delta)))
		{
			delta += signal[j].delta;
			j++;
		}

		signal[k].time = signal[i].time;
		signal[k].delta = delta;
		signal[k].value = signal[i].value;
		k ++;
		i = j-1;
	}

	signal = (struct signal_elem *)realloc(signal, sizeof(struct signal_elem) * k);

	*signalOut = signal;
	return k;
}

double get_SignalTimeLength(struct signal_elem *s, signalSize len)
{
    if (len > 0) return s[len-1].time - s[0].time;
    else return 0;
}

long long int get_DynamicSamplingRate(struct signal_elem *s, signalSize len, int num_points)
{
    return get_SignalTimeLength(s, len) / num_points;
}


double compareSignals(struct signal_elem *s1, signalSize s1_len, struct signal_elem *s2, signalSize s2_len, int windowing)
{
	FILE *gp1, *gp2;
	int x1, x2, x3;
	double y1, y2, y3, similarity;
	char *sig1_shift = "comp_sig1_shift.txt", *sig2_shift = "comp_sig2_shift.txt";
	char *sig1_samp = "comp_sig1_samp.txt", *sig2_samp = "comp_sig2_samp.txt";
	char *autocross_sig1 = "autocross_sig1.txt", *autocross_sig2 = "autocross_sig2.txt", *cross_sig1_sig2 = "cross_sig1_sig2.txt";
	struct signal_elem *sig1, *sig2;
	signalSize sig1_len, sig2_len;
    long long int samplingRate;

	/* XXX Input signals are copied because shifting and windowing modifies them (deleted at exit) */
	sig1_len = copySignal( s1, s1_len, &sig1 );
	sig2_len = copySignal( s2, s2_len, &sig2 );

	shiftSignal(sig1, sig1_len);
	shiftSignal(sig2, sig2_len);

	if (windowing > W_NONE)
	{   
	    applyWindowing(sig1, sig1_len, windowing);
	    applyWindowing(sig2, sig2_len, windowing);
	}

	dumpSignal(sig1_shift, sig1, sig1_len);
	dumpSignal(sig2_shift, sig2, sig2_len);

    samplingRate = MAX( get_DynamicSamplingRate(s1, s1_len, 1000), get_DynamicSamplingRate(s2, s2_len, 1000)  );
    fprintf(stderr, "samplingRate = %lld\n", samplingRate);

//	Sampler(sig1_shift, sig1_samp, (long long int)5000000);
//	Sampler(sig2_shift, sig2_samp, (long long int)5000000);
	Sampler(sig1_shift, sig1_samp, samplingRate);
	Sampler(sig2_shift, sig2_samp, samplingRate);

	Crosscorrelation(sig1_samp, sig2_samp, cross_sig1_sig2);

	Crosscorrelation(sig1_samp, sig1_samp, autocross_sig1);
	Crosscorrelation(sig2_samp, sig2_samp, autocross_sig2);

    gp2 = fopen(autocross_sig1, "r");
    fscanf(gp2, "%d %lf ", &x1, &y1);
    fclose(gp2);

    gp2 = fopen(autocross_sig2, "r");
    fscanf(gp2, "%d %lf ", &x2, &y2);
    fclose(gp2);

    gp1 = fopen(cross_sig1_sig2, "r");
    fscanf(gp1, "%d %lf ", &x3, &y3);
    fclose(gp1);

	similarity = sqrt(pow(y3,2)/(y1*y2));

    /* x -- shifting, punt on s1 es mes similar a s2 
       y -- crosscorrelation */

	fprintf(stderr, "y3/(y1*y2)=%f\n", similarity);

	destroySignal (sig1);
	destroySignal (sig2);

	return similarity;
}

double compareFileSignals(char * sig1_txt, char * sig2_txt, int windowing)
{
    struct signal_elem *sig1, *sig2;
    signalSize sig1_len, sig2_len;

    sig1_len = loadSignal(sig1_txt, &sig1);
    sig2_len = loadSignal(sig2_txt, &sig2);

    return compareSignals(sig1, sig1_len, sig2, sig2_len, windowing);
}


void destroySignal(struct signal_elem *signal)
{
	if (signal != NULL) free(signal);
}


int dumpSignal(char *fileName, struct signal_elem *signal, signalSize signal_size)
{
   int i;
   FILE *fd;

   if((fd = fopen(fileName, "w")) == NULL)
	return -1;

   for(i = 0; i < signal_size; i++)
	fprintf(fd, "%lld %lld %lld\n", signal[i].time, signal[i].delta, signal[i].value);

   fclose(fd);

   return 0;
}

int loadSignal(char *fileName, struct signal_elem **signalIO)
{
	FILE *fp;
    char line[LINE_MAX];
	struct signal_elem *loadedSignal;
	int cur_line = 0;
	long long int *times=NULL, *deltas=NULL, *values=NULL;
	signalSize sig_size;
	
	if ((fp = fopen(fileName, "r")) == NULL)
		return -1;


    cur_line = 0;
    while (fgets(line, LINE_MAX, fp) != NULL)
    {
		cur_line ++;
	}
	sig_size = cur_line;

	times = (long long int *)malloc(sizeof(long long int) * sig_size);
	deltas = (long long int *)malloc(sizeof(long long int) * sig_size);
	values = (long long int *)malloc(sizeof(long long int) * sig_size);

	rewind(fp);

	cur_line = 0;
    while (fgets(line, LINE_MAX, fp) != NULL)
	{
		long long int time, delta, value;	
		if (sscanf(line, "%lld %lld %lld", &time, &delta, &value) != 3)	
			return -1;

		times[cur_line] = time;
		deltas[cur_line] = delta;
		values[cur_line] = value;
		cur_line ++;
	}

	sig_size = generateSignal(times, deltas, values, sig_size, &loadedSignal);
	*signalIO = loadedSignal;

	free(times);
	free(deltas);
	free(values);

	return sig_size;
}

signalSize addSorting(int numSignals, struct signal_elem ** listSignals, signalSize * listSizes, struct signal_elem **addedSignal)
{
	int maxAddedSize = 0;
	int curSignal = 0;
	int count = 0;
	int i = 0;
	struct burst_info *burst_times = NULL;
	signalSize addedSize = 0;

	for (curSignal=0; curSignal<numSignals; curSignal++)
	{
		//fprintf(stderr, "[addSorting %d] size=%d\n", curSignal, listSizes[curSignal]);
		maxAddedSize += listSizes[curSignal];
	}

	burst_times = (struct burst_info *)malloc( maxAddedSize * 2 * sizeof(struct burst_info));

	for (curSignal=0; curSignal<numSignals; curSignal++)
	{
		for (i=0; i<listSizes[curSignal]; i++)
		{
			burst_times[count].time  = listSignals[curSignal][i].time;
			burst_times[count].value = listSignals[curSignal][i].value;
			count ++;

   	        burst_times[count].time  = listSignals[curSignal][i].time + listSignals[curSignal][i].delta;
   	        burst_times[count].value = -( listSignals[curSignal][i].value );
			count ++;
		} 
	}
	//fprintf(stderr, "[addSorting] numSignals=%d maxAddedSize=%d count=%d\n", numSignals, maxAddedSize, count);

	qsort(burst_times, count, sizeof(struct burst_info), qsort_cmp);

	addedSize = generateSignal2 (burst_times, count, addedSignal);
	free (burst_times);
	return addedSize;
}

signalSize generateSignal2 (struct burst_info * burst_times, int count, struct signal_elem **signalOut)
{
    struct signal_elem *newSignal = NULL;
	int newSignalSize = 0;
	int i = 0;

    if (count > 0)
    {
        /* Allocating memory for signal */
        if((newSignal = (struct signal_elem *)malloc(sizeof(struct signal_elem) * count)) != NULL)
        {
			for (i=0; i<count - 1; i++)
			{
				if (i>0) burst_times[i].value = burst_times[i].value + burst_times[i-1].value;

				if (burst_times[i+1].time - burst_times[i].time > 0)
				{
					newSignal[newSignalSize].time  = burst_times[i].time;
					newSignal[newSignalSize].delta = burst_times[i+1].time - burst_times[i].time;
					newSignal[newSignalSize].value = burst_times[i].value;
					newSignalSize ++;
				}
			}
		}
	}
	//fprintf(stderr, "[generateSignal2] count=%d newSignalSize=%d\n", count, newSignalSize);
	newSignal = (struct signal_elem *)realloc(newSignal, sizeof(struct signal_elem) * newSignalSize);

	*signalOut = newSignal;
	return newSignalSize;
}

signalSize add2Signals(struct signal_elem *signal1, signalSize signal1_size, struct signal_elem *signal2, signalSize signal2_size, struct signal_elem **signalAdded)
{

	int i, j, k;
	long long int value1 = 0, value2 = 0;
	struct signal_elem *signalOut = NULL;
	*signalAdded = signalOut;
	
	/* Check if any input signal is null */
	if((signal1_size <= 0) && (signal2_size <= 0))
		return 0;
	else if (signal1_size <= 0)
		return copySignal(signal2, signal2_size, signalAdded);
	else if (signal2_size <= 0)
		return copySignal(signal1, signal1_size, signalAdded);

	/* Both signals are not null */

	if((signalOut = (struct signal_elem *)malloc(sizeof(struct signal_elem)*(signal1_size + signal2_size))) == NULL)
		return 0;


	i = 0; j = 0; k = 0;

	while ((i < signal1_size) || (j < signal2_size))
	{

		if (i >= signal1_size)
		{
			value1 = 0;
			value2 = signal2[j].value;
			signalOut[k].time = signal2[j].time;
			j++;
		}
		else if (j >= signal2_size)
		{
			value1 = signal1[i].value;
			value2 = 0;
			signalOut[k].time = signal1[i].time;
			i++;
		}
		else
		{
			if (signal1[i].time < signal2[j].time)
			{
				value1 = signal1[i].value;
				signalOut[k].time = signal1[i].time;
				i++;
			}
			else if (signal1[i].time > signal2[j].time)
			{
				value2 = signal2[j].value;
				signalOut[k].time = signal2[j].time;
				j++;
			}
			else
			{
				value1 = signal1[i].value;
				value2 = signal2[j].value;
				signalOut[k].time = signal1[i].time;
				i++; j++;
			}
		}
		signalOut[k].value = value1 + value2;
		if(k > 0)
			signalOut[k-1].delta = signalOut[k].time - signalOut[k-1].time;

		k++;
	}

	/* Last delta */
	if(signalOut[k-1].time == signal1[i-1].time)
		signalOut[k-1].delta = signal1[i-1].delta;
	else
		signalOut[k-1].delta = signal2[j-1].delta;

	*signalAdded = signalOut;

	return k;
}


signalSize addNSignals(struct signal_elem **signals, signalSize *signals_size, int num_signals, struct signal_elem **signalAdded)
{
	int i, signal1_size, signal2_size, result_size;
	struct signal_elem *signal1, *signal2, *result;

	if (num_signals <= 0) 
	{
		result = NULL;
		result_size = 0;
	}
	else if (num_signals == 1) 
	{
		result_size = copySignal(signals[0], signals_size[0], &result);
	}
	else
 	{ 
		signal1 = signals[0];
		signal1_size = signals_size[0];

		for(i = 1; i < num_signals; i++)
		{
			signal2 = signals[i];
			signal2_size = signals_size[i];

			result_size = add2Signals(signal1, signal1_size, signal2, signal2_size, &result);
			if (i > 1) destroySignal(signal1);
			signal1 = result;
			signal1_size = result_size;
		} 
	}
	*signalAdded = result;
	return result_size; 
}   


/* Shifts signal to start at 0, otherwise the analysis detects less and less iters as time moves forward */
long long int shiftSignal(struct signal_elem *signal, signalSize size)
{
	int i = 0;
	long long int timeShift = 0;

	if (size > 0)
	{
		timeShift = signal[0].time;

		for (i=0; i<size; i++)
		{
			signal[i].time -= timeShift;
		}
	}
	return timeShift;
}

void chopSignal (struct signal_elem *signal, signalSize size, long long int t1, long long int t2, struct signal_elem **signal_out, signalSize * size_out)
{
	int i, t1idx = -1;
	struct signal_elem *chop = NULL;
	signalSize chop_size;

	chop_size = 0;
	for (i=0; i<size; i++)
	{
		if (signal[i].time >= t1)
		{
			if (t1idx == -1) t1idx = i;

			if (signal[i].time > t2) break;
			chop_size ++;
		}
	}

fprintf(stderr, "===> size=%d t1=%lld t2=%lld t1idx=%d chop_size=%d\n", size, t1, t2, t1idx, chop_size);

	if (chop_size > 0)
	{
		chop = (struct signal_elem *)malloc( chop_size * sizeof(struct signal_elem) );
		memcpy ( chop, &(signal[t1idx]), chop_size * sizeof(struct signal_elem));
	}
	*signal_out = chop;
	*size_out = chop_size;
}

int ExecuteAnalysis(struct signal_elem *orig_signal, signalSize orig_size, int num_iters, int windowing, char *outPrefix, Period_t ***ioPeriods)
{
	int pfound;
	long int period;
	char *signalout = "/dev/null";
	long int t0, t1;
	FILE *out; 
	FILE *err; 
	double f1, f2;
	FILE * gp;
	long long int c, d;
	long long int totaltime;
	int numPeriods = 0;
	Period_t ** listPeriods = NULL, * foundPeriod = NULL;
	STRING(tmp_dirname); STRING(siglog_out); STRING(siglog_err);
	STRING(signal_txt); STRING(signal_samp_txt); STRING(PPP2);
	long long int timeShift = 0;
	struct signal_elem *signal;
	signalSize size;

	/* XXX Input signals is copied because shifting and windowing modifies it (deleted at exit) */
	size = copySignal(orig_signal, orig_size, &signal); 

	TMPPATH(tmp_dirname, outPrefix);

	mkdir(tmp_dirname, 0700);

	TMPFILE(siglog_out, tmp_dirname, "siglog.out");
	TMPFILE(siglog_err, tmp_dirname, "siglog.err");

	out = fopen(siglog_out, "w");
	err = fopen(siglog_err, "w");

	timeShift = shiftSignal(signal, size);
	fprintf(out, "Signal shifted %lld ns\n", timeShift);

	totaltime = (signal[size-1].time + signal[size-1].delta) - signal[0].time;

	TMPFILE(signal_txt, tmp_dirname, "signal.txt");
	TMPFILE(signal_samp_txt, tmp_dirname, "signal.samp.txt");

	//BarlettWindowing(signal, size);
	if (windowing > W_NONE)
	{
		applyWindowing(signal, size, windowing);
	}

	dumpSignal (signal_txt, signal, size);

	Sampler_wavelet(signal_txt, signal_samp_txt, 1024); /* 1024 o 4096 */

	TMPFILE(PPP2, tmp_dirname, "PPP2");
	Wavelet(signal_samp_txt, PPP2);
 
	gp = fopen(PPP2, "r");

	while(fscanf(gp, "%lf %lf\n", &f1, &f2)==2) 
	{
		if(f1<f2) 
		{
			c=signal[0].time + totaltime*f1;
			d=signal[0].time + totaltime*f2;
//Analysis("signal.txt", c*1000000, d*1000000, (d-c)*1000000, c, d, 0, 0, 0, NULL, 
fprintf(stderr, "call Analysis f1=%lf f2=%lf\n", f1, f2);
			foundPeriod = Analysis(signal_txt, c/1000000, d/1000000, (d-c)/1000000, c/1000000, d/1000000, 0, 0, 0, NULL,
				NULL, 1, 0, 
				NULL, NULL, &pfound, &period, signalout,
				&t0, &t1, NULL, out, err, 0, tmp_dirname, num_iters);
fprintf(stderr, "return Analysis\n");
			if (foundPeriod != NULL)
			{
				numPeriods ++;
				listPeriods = realloc(listPeriods, numPeriods * sizeof(Period_t *));
				listPeriods[numPeriods-1] = foundPeriod;

				/* Reapply the time shift to the results */
				foundPeriod->ini += timeShift;
				foundPeriod->end += timeShift;
				foundPeriod->best_ini += timeShift;
				foundPeriod->best_end += timeShift;

				fprintf(stdout, "PERIOD %d: iters=%f length=%lld g=%lf g2=%lf g3=%lf ini=%lld end=%lld best_ini=%lld best_end=%lld\n", 
					numPeriods,
					foundPeriod->iters,
					foundPeriod->length,
					foundPeriod->goodness,
					foundPeriod->goodness2,
					foundPeriod->goodness3,
					foundPeriod->ini,
					foundPeriod->end,
					foundPeriod->best_ini,
					foundPeriod->best_end);

			}
		}
	}
	fprintf(stdout, "%d period(s) found.\n", numPeriods);

	fclose(out);
	fclose(err);

	/* Delete output files 
	char cmd[256];
	snprintf(cmd, sizeof(cmd), "rm -r %s", tmp_dirname);
	system(cmd);
	*/
	
	destroySignal(signal);

	*ioPeriods = listPeriods;
	return numPeriods;
}


