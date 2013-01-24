
ifeq "$(EXTRAE_HOME)" ""
	EXTRAE_HOME=/opt/cepbatools/extrae
endif

cutter_laptop: 
	gcc -o cutter -lm -lz -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE cutter_2.7.c
cutter_laptop_lib:
	gcc -o cutter.o -c cutter_2.7.c -D cutter_lib -lm -lz -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
cutter_laptop_lib_trace:
	gcc -o cutter.o -c cutter_2.7.c -D cutter_lib -g -D TRACE_MODE -lm -lz -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -L $(EXTRAE_HOME)/lib -lseqtrace -lxml2 -L/usr/lib64 -lbfd -L/usr/lib64 -liberty
cutterOMPSs_laptop:
	mcc --ompss -o cutter.o -c cutter_2.7.c -lm -lz -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
cutterOMPSs_laptop_lib:
	mcc --ompss -o cutter.o -c cutter_2.7.c -D cutter_lib -lm -lz -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
cutterOMPSs_laptop_lib_trace:
	mcc --ompss --instrument -o cutter.o -c cutter_2.7.c -D cutter_lib -g -lm -lz -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE



cutter_MN:
	xlc -o cutter -q64 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -lm -lz cutter_2.7.c
cutter_MN_lib:
	xlc -o cutter.o -c cutter_2.7.c -D cutter_lib -q64 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -lm -lz
cutter_MN_lib_trace:
	xlc -o cutter.o -c cutter_2.7.c -D cutter_lib -g -D TRACE_MODE -q64 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -lm -lz -L $(EXTRAE_HOME)/lib -lseqtrace -lxml2 -L/usr/lib64 -lbfd -L/usr/lib64 -liberty
cutterOMPSs_MN_lib:
	mcc --ompss -o cutter.o -c cutter_2.7.c -g -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -lm -lz
cutterOMPSs_MN_lib:
	mcc --ompss -o cutter.o -c cutter_2.7.c -D cutter_lib -g -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -lm -lz
cutterOMPSs_MN_lib_trace:
	mcc --ompss --instrument -o cutter.o -c cutter_2.7.c -D cutter_lib -g -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -lm -lz

clean:
	rm -f *.o *~ cutter

