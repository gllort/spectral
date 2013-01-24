
ifeq "$(EXTRAE_HOME)" ""
	EXTRAE_HOME=/opt/cepbatools/extrae
endif

trace_filter_MN:
	xlc -o trace_filter -q64 trace_filter_OMP.1.2.c -lm
trace_filterOMP_MN:
	xlc -o trace_filter -q64 -qsmp=omp -DOMP_FLAG -qthreaded trace_filter_OMP.1.2.c -lm
trace_filterOMPSs_MN:
	mcc --ompss -o trace_filter -DOMPSS_FLAG trace_filter_OMP.1.2.c -lm
trace_filter_MN_lib:
	xlc -o trace_filter -q64 -D trace_filter_lib trace_filter_OMP.1.2.c -lm
trace_filterOMP_MN_lib:
	xlc -o trace_filter -q64 -qsmp=omp -DOMP_FLAG -qthreaded -D trace_filter_lib trace_filter_OMP.1.2.c -lm
trace_filterOMPSs_MN_lib:
	mcc --ompss -o trace_filter -DOMPSS_FLAG -D trace_filter_lib trace_filter_OMP.1.2.c -lm
trace_filter_MN_lib_trace:
	xlc -o trace_filter -q64 -D trace_filter_lib -g -D TRACE_MODE trace_filter_OMP.1.2.c -lm -L $(EXTRAE_HOME)/lib -lseqtrace -lxml2 -L/usr/lib64 -lbfd -L/usr/lib64 -liberty
trace_filterOMP_MN_lib_trace:
	xlc -o trace_filter -q64 -qsmp=omp -DOMP_FLAG -qthreaded -D trace_filter_lib -g -D TRACE_MODE trace_filter_OMP.1.2.c -lm -L $(EXTRAE_HOME)/lib -lseqtrace -lxml2 -L/usr/lib64 -lbfd -L/usr/lib64 -liberty
trace_filterOMPSs_MN_lib_trace:
	mcc --ompss --instrument -o trace_filter -DOMPSS_FLAG -D trace_filter_lib -g -D TRACE_MODE trace_filter_OMP.1.2.c -lm




trace_filter_laptop:
	gcc -o trace_filter -lm -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE trace_filter_OMP.1.2.c
trace_filterOMP_laptop:
	gcc -fopenmp -o trace_filter -lm -DOMP_FLAG -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE trace_filter_OMP.1.2.c
trace_filterOMPSs_laptop:
	mcc --ompss -o trace_filter -lm -DOMPSS_FLAG -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE trace_filter_OMP.1.2.c
trace_filter_laptop_lib:
	gcc -o trace_filter.o -c trace_filter_OMP.1.2.c -lm -D trace_filter_lib -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
trace_filterOMP_laptop_lib:
	gcc -fopenmp -o trace_filter.o -c trace_filter_OMP.1.2.c -lm -DOMP_FLAG -D trace_filter_lib -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
trace_filterOMPSs_laptop_lib:
	mcc --ompss -o trace_filter.o -c trace_filter_OMP.1.2.c -lm -DOMPSS_FLAG -D trace_filter_lib -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
trace_filter_laptop_lib_trace:
	gcc -o trace_filter.o -c trace_filter_OMP.1.2.c -lm -g -D trace_filter_lib -D TRACE_MODE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -L $(EXTRAE_HOME)/lib -lseqtrace -lxml2 -L/usr/lib64 -lbfd -L/usr/lib64 -liberty
trace_filterOMP_laptop_lib_trace:
	gcc -fopenmp -o trace_filter.o -c trace_filter_OMP.1.2.c -lm -DOMP_FLAG -g -D trace_filter_lib -D TRACE_MODE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -L $(EXTRAE_HOME)/lib -lseqtrace -lxml2 -L/usr/lib64 -lbfd -L/usr/lib64 -liberty
trace_filterOMPSs_laptop_lib_trace:
	mcc --ompss --instrument -o trace_filter.o -c trace_filter_OMP.1.2.c -lm -DOMPSS_FLAG -g -D trace_filter_lib -D TRACE_MODE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE

clean:
	rm -f *.o *~ trace_filter
