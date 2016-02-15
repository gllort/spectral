#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include "wavelet.h"
#include "optim.h"
#include "optim-macros.h"
#include "optim-functions.h"
#include "config.h"
#if defined(HAVE_LIBTOOLS)
#include "reconstruct_trace.h"
#endif

void ParseTraceHeader(
  char          *trace, 
  long long int *total_time, 
  int           *num_threads, 
  long long int *size)
{
  FILE *fd = NULL;
  char str[65536]; /* XXX Won't be enough at large scale! */

  fd = fopen(trace, "r");
  if (fd == NULL)
  {
    fprintf(stderr, "ParseTraceHeader:: Error opening trace '%s'!\n", trace);
    perror("fopen: ");
    exit(-1);
  }
  fgets(str, sizeof(str), fd);

  sscanf(str, "%*s %*[^:]:%*[^:]:%lld", total_time );

  sscanf(str, "%*s %*[^:]:%*[^:]:%*[^:]:%*[^:]:%*[^:]:%d", num_threads );

  fseek(fd, 0L, SEEK_END);
  *size = ftello64(fd);

  fclose(fd);
}

void PrintUsage(char *app_name) 
{
  fprintf(stdout, "\nSYNTAX\n"
                  "  %s [OPTIONS] <trace1.prv> <trace2.prv> ... <METRIC>\n\n"
                  "OPTIONS\n"
                  "  Choose one between:\n"
                  "    -i <# iterations> : Generate a chop containing the given number of iterations.\n"
                  "    -s <size in Mb>   : Generate a chop of the given size.\n"
#if defined(HAVE_LIBTOOLS)
                  "    -r                : Reconstruct the trace marking the iterations.\n"
#endif
                  "\n"
                  "METRICS\n"
                  "  Choose one between:\n"
                  "    BW, MPIp2p, CPUBurst, CPUDurBurst, IPC\n\n", basename(app_name));
}

void ParseArgs(int argc, char **argv, int *out_num_traces, char ***out_input_traces, char **out_analysis_type, int *out_target_iterations, int *out_reconstruct)
{
  int i                 = 0;
  int j                 = 1;
  int target_iterations = 0;
  int use_iterations    = 0;
  int reconstruct_trace = 0;
  int target_size       = 0;
  int use_size          = 0;
  int num_traces        = 0;
  char **input_traces   = NULL;
  char *analysis_type   = NULL;
  char str[32];

  if ( ( argc == 1) ||
       ((argc == 2) &&
       (strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)))
  {
    PrintUsage(argv[0]);
    exit(EXIT_SUCCESS);
  }

  if (argv[1][0] == '-')
  {
    for (j = 1; (j < argc - 1) && (argv[j][0] == '-'); j++)
    {
      switch (argv[j][1])
      {
        case 'i':
          j++;
          target_iterations = atoi(argv[j]);
          use_iterations = 1;
          break;
#if defined(HAVE_LIBTOOLS)
        case 'r':
          reconstruct_trace = 1;
          break;
#endif
        case 's':
          j++;
          target_size = atoi(argv[j]);
          use_size = 1;
          sprintf(str, "%lld", target_size * 1000 * 1000);
          setenv("SPECTRAL_TRACE_SIZE", str, 1);
          target_iterations = 0;
          break;
        default:
          fprintf(stderr, "\n*** ERROR: Invalid parameter %s\n", argv[j]);
          PrintUsage(argv[0]);
          exit(EXIT_FAILURE);
          break;
      }
    }
  }

  if (use_iterations && use_size)
  {
    fprintf(stderr, "\n*** ERROR: Please specify either -i or -s options, but not both!\n");
    PrintUsage(argv[0]);
    exit(EXIT_FAILURE);
  }

  num_traces = argc - j - 1;

  input_traces = (char **)malloc(sizeof(char *) * num_traces);

  for (i = 0; i < num_traces; i ++)
  {
    input_traces[i] = argv[j + i];
  }

  analysis_type = argv[argc - 1];

  if ((strcmp(analysis_type, "BW") != 0) &&
      (strcmp(analysis_type, "IPC") != 0) && 
      (strcmp(analysis_type, "MPIp2p") != 0) && 
      (strcmp(analysis_type, "CPUBurst") != 0) && 
      (strcmp(analysis_type, "CPUDurBurst") != 0) && 
      (strcmp(analysis_type, "IPC") != 0))
  {
    fprintf(stderr, "\n*** ERROR: Signal requested '%s' is not a valid signal.\n", analysis_type);
    PrintUsage(argv[0]);
    exit(EXIT_FAILURE);
  }

  *out_num_traces        = num_traces;
  *out_input_traces      = input_traces;
  *out_analysis_type     = analysis_type;
  *out_target_iterations = target_iterations;
  *out_reconstruct       = reconstruct_trace;
}

int
main (int argc, char *argv[])
{
#ifdef TRACE_MODE
  Extrae_init ();
  Extrae_user_function (1);
#endif
  spectral_time_t *t0_flushing = NULL;
  spectral_time_t *t1_flushing = NULL;
  long int temps = time (NULL);
  long long int *periods = NULL;
  long long int c, d, t0, t1;
  long long int a, b, *totaltime, min, a2, b2;
  int *p;
  int j, i, k, trace_num, change, num_chop=0;
  char **signals, **traces;
  char filtered_trace[1024];
  FILE *hp, *jp;

#ifdef DEBUG_MODE
  FILE *fDB1;
#endif
  int num_flushes = 0;
  int num_traces = 0;
  char **input_traces = NULL;
  char *analysis_type = NULL;
  int target_iterations = 0;
  int reconstruct_trace = 0;
  Period_t **detected_periods = NULL;
  int num_detected_periods = 0;

  ParseArgs(argc, argv, &num_traces, &input_traces, &analysis_type, &target_iterations, &reconstruct_trace);

  periods   = (long long int *) malloc (sizeof (long int) * num_traces);
  totaltime = (long long int *) malloc (sizeof (long long int) * num_traces);
  p         = (int *)           malloc (sizeof (int) * num_traces);
  signals   = (char **)         malloc (sizeof (char *) * num_traces);
  traces    = (char **)         malloc (sizeof (char *) * num_traces);

  for (i = 0; i < num_traces; i ++)
  {
    signals[i] = (char *) malloc (sizeof (char) * 256);
    traces[i]  = (char *) malloc (sizeof (char) * 256);
  }

  change = 0;

  /* Create the output reports */
  hp = fopen ("report.out", "w");
  jp = fopen ("report.err", "w");


  /* Loop of the tracefiles */
  for (trace_num = 0; trace_num < num_traces; trace_num++)
  {
    long long int trace_total_time  = 0;
    int           trace_num_threads = 0;
    long long int trace_size        = 0;
    int           periods_found     = 0;

    /* Extraction of the execution time and the number of threads */
    ParseTraceHeader( input_traces[trace_num], &trace_total_time, &trace_num_threads, &trace_size );

    totaltime[trace_num] = trace_total_time;
    p[trace_num]         = trace_num_threads;

    /* Prepare outpuf files */
    sprintf (signals[trace_num], "%s.1it.txt", input_traces[trace_num]);
    sprintf (filtered_trace, "%s.%s.filtered.prv", input_traces[trace_num], analysis_type);

    fprintf (hp, "Trace: %s\n", input_traces[trace_num]);
    fprintf (hp, "Metric: %s\n", analysis_type);
    fprintf (hp, "Total time=%lld ms\n", totaltime[trace_num] / 1000000);
    fprintf (hp, "Total threads=%d\n", p[trace_num]);
    fprintf (hp, "Size=%lld\n\n", trace_size);

    /* Generate the flushing signal */
    signal_t *signal_flushing = NULL;
    

    fprintf(hp, "\nGenerating the flushing signal..."); fflush(stdout);
    signal_flushing = Generate_Event_Signal(input_traces[trace_num], 40000003, (char *)"signal_flush.txt");
    fprintf(hp, "done!\n");

    /* Get the boundaries of the flushing regions */
    num_flushes = GetBoundaries(signal_flushing, totaltime[trace_num], &t0_flushing, &t1_flushing, (char *)"boundaries.txt");

 
    /* Generate the signal we will study */
    fprintf (hp, "Filtered trace: %s.%s.filtered.prv\n", input_traces[trace_num], analysis_type);

    spectral_value_t *wavelet_samples = NULL;
    int samples_to_take = 0;
    signal_t *signal_to_analyze = NULL;
    signal_t *signal_to_sample  = NULL;
   

    signal_t *signal_in_running  = NULL;
    signal_t *signal_dur_running = NULL;
    int       num_periodical_regions_without_flushing = 0;


      






    if (strcmp(analysis_type, "BW") == 0) 
    {
      signal_to_analyze = Generate_BW_Signal( input_traces[trace_num], (char *)"signal.txt" );
      signal_to_sample  = signal_to_analyze;
      samples_to_take   = 1024;
    }
    else if (strcmp(analysis_type, "IPC") == 0)
    {
      signal_to_analyze = Generate_IPC_Signal( input_traces[trace_num], filtered_trace, (char *)"signal.txt" );
      signal_to_sample  = signal_to_analyze;
      samples_to_take   = 4096;
    }
    else if (strcmp(analysis_type, "MPIp2p") == 0)
    {
      signal_to_analyze = Generate_MPIp2p_Signal( input_traces[trace_num], filtered_trace, (char *)"signal.txt" );
      signal_to_sample  = signal_to_analyze;
      samples_to_take   = 4096;
    }
    else if (strcmp(analysis_type, "CPUBurst") == 0)
    {
      Generate_Running_Signals( input_traces[trace_num], filtered_trace, totaltime[trace_num]/10000000, 
        &signal_in_running, (char*)"signal_in_running.txt", &signal_dur_running, (char *)"signal_dur_running.txt" );
      signal_to_sample  = signal_to_analyze = signal_in_running;
      samples_to_take   = 4096;
    }
    else if (strcmp(analysis_type, "CPUDurBurst") == 0)
    {
      Generate_Running_Signals( input_traces[trace_num], filtered_trace, totaltime[trace_num]/100000,
        &signal_in_running, (char *)"signal_in_running.txt", &signal_dur_running, (char *)"signal_dur_running.txt" );
      signal_to_sample  = signal_in_running;
      signal_to_analyze = signal_dur_running;
      if (trace_size<10000000000LL) { samples_to_take = 4096; }
      else { samples_to_take = 655360; }
    }
    else
    {
      fprintf(stderr, "Signal requested '%s' is not a valid signal. "
                      "Please choose one between: BW, IPC, MPIp2p, CPUBurst or CPUDurBurst.\n", 
                      analysis_type);
      exit(-1);
    }

    if ((signal_in_running == NULL) || (signal_dur_running == NULL)) /* BW, IPC, MPIp2p2 cases */
    {
      /* Generate running burst signal anyway to obtain a right cut of the traces */
      Generate_Running_Signals( input_traces[trace_num], filtered_trace, totaltime[trace_num]/10000000,
        &signal_in_running, "signal_in_running.txt", &signal_dur_running, (char *)"signal_dur_running.txt" );        
    }
    
    /* Global analysis using wavelets */

    Sampler_wavelet( signal_to_sample, &wavelet_samples, samples_to_take, (char *)"signal_sampled.txt" );

    /* XXX Change wavelet so that returns an array of periodical regions without flushing (struct with f1, f2 fields) and a count, 
     * instead of reusing the input wavelet_samples vector!!! */
    Wavelet( wavelet_samples, samples_to_take, &num_periodical_regions_without_flushing, (char *)"wavelet.txt" );

    /* Selection of the peridical regions without flushing */
    
    for (i = 0; i < num_periodical_regions_without_flushing; i += 2)
    {
      double f1 = wavelet_samples[i];
      double f2 = wavelet_samples[i + 1];
      
      if (f1 < f2)
      {
        c  = totaltime[trace_num]/1000000*f1;
        d  = totaltime[trace_num]/1000000*f2;
        a  = 0;
        b  = 0;
        a2 = 0;
        b2 = 0;
        fprintf( jp, "From "FORMAT_TIME" to "FORMAT_TIME" ms\n", c, d );
        fprintf( stdout, "From "FORMAT_TIME" to "FORMAT_TIME" ms\n", c, d );
  
        min = j = k = 0;

        for (j = 0; j <= num_flushes; j ++)
        {
          if ( (c < t0_flushing[j]) && (d > t1_flushing[j]) )
          {
            if ( min < (t1_flushing[j] - t0_flushing[j]) ) 
            {
              a   = t0_flushing[j];
              b   = t1_flushing[j];
              a2  = t0_flushing[j+3];
              b2  = t1_flushing[j+3];
              min = b - a;
              k   = j;
            }
          }
          if ( (t0_flushing[j] <= c) && (d > t1_flushing[j]) )
          {
            if ( min < (t1_flushing[j] - c) )
            {
              a   = c;
              b   = t1_flushing[j];
              min = b - a ;
              k   = j;
            }
          }
          if ( (c < t0_flushing[j]) && (t0_flushing[j] <= d) && (t1_flushing[j] >= d) ) 
          {
            if ( min < (d - t0_flushing[j]) )
            {
              a   = t0_flushing[j];
              b   = d;
              min = b - a;
              k   = j;
            }
          }
          if ( (t0_flushing[j] <= c) && (t1_flushing[j] >= d) )
          {
            if ( min < (d - c) )
            {
              a   = c;
              b   = d;
              min = b - a;
              k   = j;
            }
          }
        }

        fprintf(jp, "%lld %lld\n", a, b);

        /* Analysis of the regions */
        if ( (b != 0) && ((d - c) > 0.05*totaltime[trace_num]/1000000) )
        {
          Period_t *cp = NULL;

          num_chop++;
          fprintf(hp, "\n");
          fprintf(stdout, "\nExecuting analysis... ");
          fflush(stdout);
          cp = Analysis(signal_to_analyze, a, b, d-c, c, d, 
                        analysis_type, input_traces[trace_num], 1, signal_dur_running,
                        &periods_found, &periods[trace_num], signals[trace_num], &t0, &t1, traces[trace_num], hp, jp, num_chop, target_iterations, trace_size,
                        totaltime[trace_num]);

#if defined(HAVE_LIBTOOLS)
          if (reconstruct_trace)
          {
            detected_periods = (Period_t **)malloc((num_detected_periods + 1) * sizeof(Period_t *));
            detected_periods[num_detected_periods] = cp;
            num_detected_periods ++;
          }
#endif
        }
      }
    }
    if (!periods_found) 
    {
      fprintf(stdout, "No periods found for trace %s\n", input_traces[trace_num]);
    }

#if defined(HAVE_LIBTOOLS)
    if (reconstruct_trace)
    {
      fprintf(stdout, "\nReconstructing trace... ");
      fflush(stdout);
      Reconstruct((char *)input_traces[trace_num], num_detected_periods, detected_periods);
    }
#endif
  }
    
  fprintf (hp, "Speedup Analysis\n");
  for (trace_num = 0; trace_num < num_traces; trace_num ++)
    {
      fprintf (hp, "Trace %d : %f\n", trace_num+1,
	       (1.0 * periods[0]) / (1.0 * periods[trace_num]));

    }

  fclose (hp);
  fclose (jp);

  fprintf (stdout, "Success!!!");

temps = time(NULL) - temps;
fprintf(stdout, "%li Hours, %li Minutes, %li Seconds\n", temps/3600, (temps%3600)/60,(temps%3600)%60);

#ifdef TRACE_MODE
  Extrae_user_function (0);
  Extrae_fini ();
#endif


  return 0;
}
