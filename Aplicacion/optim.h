#include "signal_interface.h"
#include "optim-functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef char* file;
	
	Period_t * Analysis(struct signal_info *signal,int sizeSig, long long int t0, long long int t1, long int duration, long int r0, long int r1,
char* option, file trace, int cut,struct signal_info *signal3,int sizeSig3,int* pfound,
long int* period, file signalout, long int* point1, long int* point2, file filename, FILE *out,
FILE *err, int num_chop, int requested_iters,
long long int sizeTraceOrig, long long int totaltimeTraceOrig);

#ifdef __cplusplus
}
#endif

