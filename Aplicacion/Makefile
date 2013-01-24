#Path lib trace2trace
TRACE2TRACELIBPATH=/home/user/TRACE2TRACE/lib
#path extrae
EXTRAE_HOME=/opt/extrae

# ARCHITECTURE => MN , laptop
ARCHITECTURE = laptop
BITS=64
TEMP_FILES= *.txt KK *.row gmon.out report.* *.prv *.pcf PPP2 Tcorrect *~ TRACE*.mpit* set-0
TRACE2TRACE=-L$(TRACE2TRACELIBPATH) -ltrace2trace -lz

#CC flags y allcompiles
ifeq "$(CC_MODE)" "smpss"
	CC = mcc --ompss -D OMPSS_MODE
	ALLCOMPILES = optim
	TRACE2TRACE=-L$(TRACE2TRACELIBPATH) -ltrace2traceOmpSs -lz
else
	CC = gcc
	ALLCOMPILES = optim libspectral.so libspectral.a
endif

#GPROF FLAGS
ifeq "$(GPROF)" "static"
#gprof flag compile for static
	GPROF_FLAGS=-pg -g -static

else ifeq "$(GPROF)" "nostatic"
#gprof flag compile for nostatic
	GPROF_FLAGS=-pg -g
else
	GPROF_FLAGS=
endif


#debug mode on flag for compile 
ifeq "$(DEBUG_MODE)" "1"
	DEBUG_FLAGS=-D DEBUG_MODE
else
	DEBUG_FLAGS=
endif


#trace mode on flags for compilation
ifeq "$(TRACE)" "extrae"
	TRACEPARAVERLIBS=-L $(EXTRAE_HOME)/lib -lseqtrace -lxml2 -L/usr/lib64 -lbfd -L/usr/lib64 -liberty
	TRACEPARAVER_FLAGS=-g -D TRACE_MODE
	TRACE2TRACE=-L$(TRACE2TRACELIBPATH) -ltrace2traceTraced -lz
else ifeq "$(TRACE)" "smpss"
	#TRACEPARAVERLIBS=-L $(EXTRAE_HOME)/lib -lsmpsstrace -lxml2 -L/usr/lib64 -lbfd -L/usr/lib64 -liberty
	TRACEPARAVER_FLAGS= --instrument
	TRACE2TRACE=-L$(TRACE2TRACELIBPATH) -ltrace2traceTracedOmpSs -lz
else
	TRACEPARAVERLIBS=
endif


#test mode on flag compile
ifeq "$(TEST_MODE)" "1"
	TEST_FLAGS=-D TEST_MODE
else
	TEST_FLAGS=
endif


#gdb flags for compile
ifeq "$(GDB_MODE)" "1"
	OPTIM_FLAGS=-O0 -g
else
	OPTIM_FLAGS=-O2
	#OPTIM_FLAGS=-O3
endif

#common flags
CFLAGS = $(OPTIM_FLAGS) $(GPROF_FLAGS) $(DEBUG_FLAGS) $(TEST_FLAGS) $(TRACEPARAVER_FLAGS) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE 
COMMONLIBS = -lm  $(TRACEPARAVERLIBS)

#ARCHITECTURE flags
ifeq "$(ARCHITECTURE)" "MN"
	FFTHOME = /gpfs/apps/FFTW/3.1.1/${BITS}
	INCLUDE = -I${FFTHOME}/include -I$(TRACE2TRACELIBPATH)
	FFTLIBS = -L${FFTHOME}/lib -Wl,--rpath -Wl,${FFTHOME}/lib -lfftw3
	
else ifeq "$(ARCHITECTURE)" "laptop"
	#OPTIM_FLAGS = -O3 -pipe -march=core2 -mtune=generic -mfpmath=sse -ffloat-store -Wl,-s,-O1 -lm
	INCLUDE = -I/usr/include -I./ -I$(TRACE2TRACELIBPATH)
	FFTLIBS = -lfftw3
endif

####################################################


#all: optim libspectral.so libspectral.a
all: $(ALLCOMPILES)

clean:
	rm -rf *.o optim libspectral.so libspectral.a $(TEMP_FILES)


LIB_SOURCES = optim.c optim-functions.c signal_interface.c 
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)

libspectral.a: $(LIB_SOURCES)
	$(CC) -c $(LIB_SOURCES) $(CFLAGS) -DLIB_MODE $(INCLUDE)
	ar -cru $@ $(LIB_OBJECTS)

libspectral.so: $(LIB_SOURCES)
	$(CC) -c $(LIB_SOURCES) $(CFLAGS) -DLIB_MODE -fPIC $(INCLUDE)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $(LIB_OBJECTS) $(FFTLIBS) $(COMMONLIBS)

optim: 
	$(CC)  -o $@ optim-functions.c optim.c SenyalD.c SenyalE.c wavelet.c fft.c fftprev.c $(CFLAGS) $(INCLUDE) $(TRACE2TRACE) $(FFTLIBS) $(COMMONLIBS)
