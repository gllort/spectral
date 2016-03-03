#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include "config.h"
#include "spectral-api.h"
#include "reconstruct_trace.h"

int main(int argc, char **argv)
{
  char *input_trace      = NULL;
  char *csv              = NULL;
  int target_iters       = 0;
  signal_t *signal       = NULL;
  FILE *fd               = NULL;
  int numPeriods         = 0;
  Period_t **listPeriods = NULL; 

  if (argc != 4)
  {
    fprintf(stderr, "Invalid arguments!\n%s <trace> <csv> <target_iterations>\n", basename(argv[0]));
    exit(-1);
  }

  input_trace  = argv[1];
  csv          = argv[2];
  target_iters = atoi(argv[3]);

  fd = fopen(csv, "r"); /* should check the result */
  if (fd == NULL) 
  {
    fprintf(stderr, "Error opening CSV file '%s'\n", csv);
    exit(-1);
  }

  signal = Spectral_AllocateSignal(1000);

  char line[256];
  while (fgets(line, sizeof(line), fd)) 
  {
    double t, d, v;

    sscanf(line, "%*d %lf %lf %lf\n", &t, &d, &v);

    spectral_time_t  time  = (spectral_time_t)  t;
    spectral_delta_t delta = (spectral_delta_t) d;
    spectral_value_t value = (spectral_value_t) v;

    Spectral_AddPoint3( signal, time, delta, value );
  }

  fclose(fd);

  fprintf(stdout, "Signal size = %d\n", Spectral_GetSignalSize(signal));

  Spectral_DumpSignal(signal, "signal.txt");

  numPeriods = Spectral_ExecuteAnalysis(signal, target_iters, WINDOWING_NONE, &listPeriods);

#if defined(HAVE_LIBTOOLS)
  Reconstruct(input_trace, numPeriods, listPeriods);
#endif

  return 0;
}
