
include_HEADERS = spectral-api.h

lib_LTLIBRARIES = libspectral.la

libspectral_la_SOURCES = \
  optim.c            optim.h                \
  optim-functions.c  optim-functions.h      \
  spectral-api.c     spectral-api.h         \
  wavelet.c          wavelet.h              \
  fft.c              fft.h                  \
  fftprev.c          fftprev.h              \
  dilation.c         dilation.h             \
  erosion.c          erosion.h              \
  cutter_2.7.c       trace_filter_OMP.1.2.c \
  trace2trace.h 

libspectral_la_CFLAGS   = -I$(FFT_INCLUDES) 
libspectral_la_LDFLAGS  = -L$(FFT_LIBSDIR) $(FFT_LIBS) -lz 

if DEBUG_MODE
libspectral_la_CFLAGS  += -g
endif

if HAVE_EXTRAE
libspectral_la_CFLAGS  += -I$(EXTRAE_INCLUDES)
libspectral_la_LDFLAGS += -L$(EXTRAE_LIBSDIR) $(EXTRAE_LIBS)
endif

if HAVE_OPENMP
libspectral_la_CFLAGS  += $(OPENMP_CFLAGS)
endif

bin_PROGRAMS = trace-analysis signal-analysis compare-signals

trace_analysis_SOURCES = \
  optim-main.c
trace_analysis_CFLAGS  = -I$(FFT_INCLUDES)
trace_analysis_LDADD   = libspectral.la

signal_analysis_SOURCES = \
  signal-analysis.c
signal_analysis_LDADD   = libspectral.la

compare_signals_SOURCES = \
  compare-signals.c
compare_signals_LDADD   = libspectral.la

bin_SCRIPTS = plot-signal
