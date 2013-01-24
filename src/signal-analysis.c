#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include "spectral-api.h"

int main(int argc, char **argv)
{
  signal_t *signal;
  int target_iters = 0;
  Period_t **listPeriods;
 
  if (argc != 3)
  {
    fprintf(stderr, "Invalid arguments!\n%s <signal.txt> <target_iterations>\n", basename(argv[0]));
    exit(-1);
  }

  signal = Spectral_LoadSignal(argv[1]);
  target_iters = atoi(argv[2]);

  Spectral_DumpSignal(signal, "0.txt");

  Spectral_ExecuteAnalysis(signal, target_iters, WINDOWING_NONE, &listPeriods);

  return 0;
}
