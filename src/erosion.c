#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "erosion.h"
#include "optim-macros.h"

int
compare_SenyalE (const void *a, const void *b)
{
  if (*(long long int *) a > *(long long int *) b)
    return 1;
  else if (*(long long int *) a == *(long long int *) b)
    return 0;
  else
    return -1;
}

spectral_value_t MinInRange (signal_t *signal, spectral_time_t p, spectral_time_t structuring_value)
{
  int j = 0;
  int k = 0;
  spectral_value_t minim = 0;

  p = p + 1;
  structuring_value = structuring_value - 1;

  while ( (j < SIGNAL_SIZE(signal)) && (p >= SIGNAL_TIME(signal, j)) )
  {
    j++;
  }
  k = MAX (0, j - 1);
  minim = SIGNAL_VALUE(signal, k);
  k = k - 1;
  while ( (k > -1) && (llabs (SIGNAL_TIME(signal, k) + SIGNAL_DELTA(signal, k) - p) <= structuring_value/2) )
  {
    minim = MIN (SIGNAL_VALUE(signal, k), minim);
    k--;
  }
  k = j;
  while ( (k < SIGNAL_SIZE(signal) - 1) && (llabs (SIGNAL_TIME(signal, k) - p) <= structuring_value/2) )
  {
    minim = MIN (SIGNAL_VALUE(signal, k), minim);
    k++;
  } 
  return minim;
}

void PassadaMin (signal_t *signal, spectral_time_t structuring_value, spectral_time_t **B1_ptr, spectral_value_t **B2_ptr)
{
  int j = 0;
  int k = 0;
  spectral_time_t  *B1 = *B1_ptr;
  spectral_value_t *B2 = *B2_ptr;

  for (j = 0; j < SIGNAL_SIZE(signal); j++)
  {
    B1[k]     = SIGNAL_TIME(signal, j) - structuring_value/2;
    B1[k + 1] = SIGNAL_TIME(signal, j);
    B1[k + 2] = SIGNAL_TIME(signal, j) + structuring_value/2;
    k += 3;
  }
  B1[k] = SIGNAL_LAST_TIME(signal) + SIGNAL_LAST_DELTA(signal);

  /* Sorting */

  qsort (B1, 3 * SIGNAL_SIZE(signal) + 1, sizeof (spectral_time_t), compare_SenyalE);

  for (j = 0; j < 3 * SIGNAL_SIZE(signal) + 1; j++)
  {
    B2[j] = MinInRange (signal, B1[j], structuring_value);
  }
}

/**
 * Builds the erosion signal that has the minimum values of the input signal within the 
 * interval defined by the structuring value.
 */
signal_t *Erosion(signal_t *signal, spectral_time_t structuring_value, char *dbg_file)
{
  int j = 0;
  int k = 0;
  int sizeB = 3 * SIGNAL_SIZE(signal) + 1;
  int sizeP = 0;
  spectral_time_t  *B1 = NULL, *temp1 = NULL, *P1 = NULL;
  spectral_value_t *B2 = NULL, *temp2 = NULL, *P2 = NULL;
  signal_t *signalE = NULL;

#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif
  
  B1 = (spectral_time_t *)  malloc(sizeof(spectral_time_t)  * sizeB);
  B2 = (spectral_value_t *) malloc(sizeof(spectral_value_t) * sizeB);

  temp1 = (spectral_time_t *)  malloc(sizeof(spectral_time_t)  * sizeB);
  temp2 = (spectral_value_t *) malloc(sizeof(spectral_value_t) * sizeB);

  PassadaMin (signal, structuring_value, &B1, &B2);

#if 0
  while (j <= sizeB - 1)
  {
    temp1[k] = B1[j];
    temp2[k] = B2[j];
    k++;
    j++;

    if (B2[j] == B2[j - 1])
    {
      while (B2[j] == B2[j - 1] & j <= sizeB - 2)
      {
        j++;
      }
      temp1[k] = B1[j - 1];
      temp2[k] = B2[j - 1];
      k++;
    }
  }
#else
  temp1[0] = B1[0];
  temp2[0] = B2[0];
  for (j = 1, k = 1; j < sizeB; j ++)
  {
    if (B2[j] != B2[j-1])
    {
      temp1[k] = B1[j]; /* XXX Originally j-1, we guess that's wrong! */
      temp2[k] = B2[j]; /* XXX Originally j-1, we guess that's wrong! */
      k ++;
    }
  }
#endif

  P1 = (spectral_time_t *)  malloc (sizeof (spectral_time_t)  * 2 * k);
  P2 = (spectral_value_t *) malloc (sizeof (spectral_value_t) * 2 * k);

  for (j = 1; j <= k; j++)
  {
    P1[2 * j - 2] = temp1[j - 1];
    P2[2 * j - 2] = temp2[j - 1];
    P1[2 * j - 1] = temp1[j - 1];
    P2[2 * j - 1] = MinInRange (signal, temp1[j - 1] + 1, structuring_value);
  }
  sizeP = 2 * k;

  signalE = Spectral_AllocateSignal( sizeP );

  int overwrite_last = 0;
  for (j = 0; j < sizeP - 1; j++)
  { 
    if (overwrite_last)
    {
      SIGNAL_SIZE(signalE) --; /* Remove the last point from the signal */
      overwrite_last = 0;
    }
    SIGNAL_ADD_POINT_3(signalE,
      P1[j],
      P1[j + 1] - P1[j],
      P2[j]
    );
    if (SIGNAL_LAST_DELTA(signalE) == 0)
    {
      overwrite_last = 1;
    }
  }

  free (B1);
  free (B2);
  free (temp1);
  free (temp2);
  free (P1);
  free (P2);

#ifdef DEBUG_MODE
  Spectral_DumpSignal(signalE, dbg_file);
#endif
#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif

  return signalE;
}

