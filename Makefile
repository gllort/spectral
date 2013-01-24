
# MN, laptop
ARCHITECTURE = MN
BITS=64

###################################################
##  MN
###################################################

ifeq "$(ARCHITECTURE)" "MN"
  CC = gcc 
  CFLAGS = -m${BITS} -O2 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -g
  COMMONLIBS = -lm
  FFTHOME = /apps/FFTW/3.3/${BITS}
  INCLUDE = -I${FFTHOME}/include
  FFTLIBS = -L${FFTHOME}/lib -Wl,--rpath -Wl,${FFTHOME}/lib -lfftw3
endif

###################################################
## laptop
###################################################

ifeq "$(ARCHITECTURE)" "laptop"
  CC = gcc
  CFLAGS = -m${BITS} -O2 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE 
  INCLUDE = -I/usr/include -I./ 
  COMMONLIBS = -lm
  FFTLIBS = -lfftw3 
endif

####################################################

PREFIX = /home/bsc41/bsc41127/APPS/SPECTRAL
BINS = optim SenyalD SenyalE GenerateSignalDurRunninglog wavelet fft fftprev
LIBS = libspectral.so libspectral.a
TARGETS = $(BINS) $(LIBS)

all: $(TARGETS)



auxiliar_binaries: SenyalD SenyalE GenerateSignalDurRunninglog wavelet fft fftprev 



clean:
	rm -rf *.o optim SenyalD SenyalE GenerateSignalDurRunninglog wavelet fft fftprev libspectral.so libspectral.a 


LIB_SOURCES = optim.c optim-functions.c signal_interface.c 
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)

libspectral.a: $(LIB_SOURCES)
	$(CC) -c $(LIB_SOURCES) $(CFLAGS) -DLIB_MODE $(INCLUDE)
	ar -cru $@ $(LIB_OBJECTS)

libspectral.so: $(LIB_SOURCES)
	$(CC) -c $(LIB_SOURCES) $(CFLAGS) -DLIB_MODE -fPIC $(INCLUDE)
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $(LIB_OBJECTS) $(FFTLIBS) $(COMMONLIBS)


optim:
	$(CC) -o $@ optim-functions.c optim.c $(CFLAGS) $(INCLUDE) $(FFTLIBS) $(COMMONLIBS)


SenyalD:
	$(CC) -o SenyalD $(CFLAGS) SenyalD.c $(COMMONLIBS)

mysenyalD:
	$(CC) -o mysenyalD $(CFLAGS) mysenyalD.c $(COMMONLIBS)


SenyalE:
	$(CC) -o SenyalE $(CFLAGS) SenyalE.c $(COMMONLIBS)



GenerateSignalDurRunninglog:
	$(CC) -o GenerateSignalDurRunninglog $(CFLAGS) GenerateSignalDurRunninglog.c $(COMMONLIBS)



wavelet:
	$(CC) -o wavelet $(CFLAGS) wavelet.c $(COMMONLIBS)


fft:
	$(CC) -o fft $(CFLAGS) $(INCLUDE) fft.c $(FFTLIBS) $(COMMONLIBS)


fftprev:
	$(CC) -o fftprev $(CFLAGS) $(INCLUDE) fftprev.c $(FFTLIBS) $(COMMONLIBS)



install: $(TARGETS)
	mkdir -p   $(PREFIX)/bin
	mkdir -p   $(PREFIX)/lib
	mkdir -p   $(PREFIX)/include
	cp $(BINS) $(PREFIX)/bin
	cp $(LIBS) $(PREFIX)/lib
	cp signal_interface.h optim-functions.h $(PREFIX)/include


