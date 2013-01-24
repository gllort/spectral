#ifndef __EROSION_H__
#define __EROSION_H__

#include "optim-functions.h"

signal_t *Erosion(signal_t *signal, spectral_time_t structuring_value, char *dbg_file);

#endif /* __EROSION_H__ */
