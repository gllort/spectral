#ifndef __RECONSTRUCT_TRACE_H__
#define __RECONSTRUCT_TRACE_H__

#include "spectral-api.h"

#if defined(__cplusplus)
extern "C" {
#endif

int Reconstruct(char *input_trace, int num_detected_periods, Period_t **detected_periods);

#if defined(__cplusplus)
}
#endif

#endif /* __RECONSTRUCT_TRACE_H__ */
