#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "wavelet.h"

void
haartransform (double *input, double *output, int n)
{

  int k;
  double i, length;
  long long int a;

  for (k = 0; k < n; k++)
    {
      output[k] = 0.0;
    }

  i = log ((double) n) / log (2);

  a = exp ((i - 1) * log (2));

  for (length = i; length > 0; length--)
    {
      i = (exp ((length - 1) * log (2)));
      for (k = 1; k <= i; k++)
	{
	  output[k - 1] = input[k * 2 - 2] + input[k * 2 - 1];	//sum
	  output[k + a - 1] = input[k * 2 - 2] - input[k * 2 - 1];	//dif
	}
      for (k = 0; k < n; k++)
	{
	  input[k] = output[k];
	}

      a = a / 2;

    }

}


void Wavelet (double *inOut, int n, int *size, char *dbg_file) 
{
	/**
	*pre: 	inOut: input data
	*	n:size input data 
	*post:	inOut: output datafile
	*	@return size output data
	**/
#ifdef TRACE_MODE
  Extrae_user_function (1);
#endif
  double *output=NULL, MAXvalue=0, SUM=0, DEV=0;
  long long int m=0, k=0, MAX=0;

  output = (double *) malloc (sizeof (double) * n);

  haartransform (inOut, output, n);

  SUM = 0;
  m = n / 2;
  for (k = m; k < n; k++)
    {
      output[k] = fabs (output[k]);
      SUM = SUM + output[k];
    }

  SUM = SUM * 2 / n;
  MAXvalue = 0;
  for (k = m; k < n; k++)
    {

      DEV = DEV + (SUM - output[k]) * (SUM - output[k]);

      if (output[k] > MAXvalue)
	{
	  MAXvalue = output[k];
	}

    }

  DEV = sqrt (DEV * 2 / n);

  MAXvalue = (2 * (SUM + 5 * DEV)) / 16;
  *size = 0;

  for (k = m; k < n; k++)
    {
      if (MAXvalue <= output[k])
	{
	  MAX = k - m;
	  inOut[*size] = (double) MAX / (double) m;

	  while ((k < n) && (MAXvalue <= output[k] || MAX + 50 > k - m))
	    {
	      if (MAXvalue <= output[k])
		{
		  MAX = k - m;
		}
	      k++;
	    }

	  inOut[*size + 1] = (double) MAX / (double) m;
	  *size = *size + 2;
	}
    }

#ifdef DEBUG_MODE
  FILE *fd = NULL;
  if (dbg_file != NULL)
  {
    fd = fopen (dbg_file, "w");
    if (fd == NULL)
    {
      fprintf(stderr, "\nWavelet:: Error: Can't open file '%s'!\n", dbg_file);
      perror("fopen: ");
      exit (-1);
    }
    k = 0;
    while (k < *size)
    {
      fprintf (fd, "%lf\n", inOut[k]);
      k++;
    }
    fclose (fd);
  }
#endif

#ifdef TRACE_MODE
  Extrae_user_function (0);
#endif
}
