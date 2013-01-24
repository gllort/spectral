#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include "spectral-api.h"

int main(int argc, char **argv)
{
  double similarity;
  signal_t *signal1;
  signal_t *signal2;

  if (argc != 3)
  {
    fprintf(stderr, "Invalid arguments!\n%s <signal_1.txt> <signal_2.txt>\n", basename(argv[0]));
    exit(-1);
  }

  signal1 = Spectral_LoadSignal(argv[1]);
  signal2 = Spectral_LoadSignal(argv[2]);

  similarity = Spectral_CompareSignals(signal1, signal2, WINDOWING_NONE);

  fprintf(stdout, "\nSignals are %.2lf %% alike\n\n", similarity * 100);

  return 0;
}
