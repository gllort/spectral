#ifndef __DILATION_H__
#define __DILATION_H__

#include "optim-functions.h"

signal_t *Dilation(signal_t *signal, spectral_time_t structuring_value, char *dbg_file);

#endif /* __DILATION_H__ */
