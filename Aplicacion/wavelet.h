#ifndef _WAVELET_H
#define _WAVELET_H

#pragma omp task input(n) inout(inOut[n]) output(size[1])
void Wavelet_exec(double *inOut,int n,int *size);

#endif /* _WAVELET_H */
