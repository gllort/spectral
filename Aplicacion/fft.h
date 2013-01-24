#ifndef _FFT_H
#define _FFT_H

void fft_exec(fftw_complex *in,int N,struct signalFreq_info *conj, long long int freq);

#endif /* _FFT_H */
