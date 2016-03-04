
include_HEADERS = spectral-api.h

lib_LTLIBRARIES = libspectral.la

libspectral_la_SOURCES  = \
  optim.c               optim.h                \
  optim-functions.c     optim-functions.h      \
  spectral-api.c        spectral-api.h         \
  wavelet.c             wavelet.h              \
  fft.c                 fft.h                  \
  fftprev.c             fftprev.h              \
  dilation.c            dilation.h             \
  erosion.c             erosion.h              \
  cutter_2.7.c          trace_filter_OMP.1.2.c \
  trace2trace.h         optim-macros.h

libspectral_la_CFLAGS   = -I$(FFT_INCLUDES) 
libspectral_la_LDFLAGS  = -L$(FFT_LIBSDIR) $(FFT_LIBS) -lz 

if DEBUG_MODE
libspectral_la_CFLAGS   += -g
endif

if HAVE_EXTRAE
libspectral_la_CFLAGS   += -I$(EXTRAE_INCLUDES)
libspectral_la_LDFLAGS  += -L$(EXTRAE_LIBSDIR) $(EXTRAE_LIBS)
endif

if HAVE_OPENMP
libspectral_la_CFLAGS   += $(OPENMP_CFLAGS)
endif

bin_PROGRAMS = trace-analysis signal-analysis csv-analysis compare-signals

trace_analysis_SOURCES = \
  optim-main.cpp
trace_analysis_CXXFLAGS = -I$(FFT_INCLUDES)
trace_analysis_LDADD    = libspectral.la
if HAVE_LIBTOOLS
trace_analysis_SOURCES += \
  reconstruct_trace.cpp reconstruct_trace.h 
trace_analysis_CXXFLAGS += `@PRVPARSER_CONFIG@ --cflags`
trace_analysis_LDFLAGS = `@PRVPARSER_CONFIG@ --libs`
endif

signal_analysis_SOURCES = \
  signal-analysis.c
signal_analysis_LDADD   = libspectral.la

csv_analysis_SOURCES = \
  csv-analysis.c
if HAVE_LIBTOOLS
csv_analysis_SOURCES += \
  reconstruct_trace.cpp reconstruct_trace.h
csv_analysis_CXXFLAGS = `@PRVPARSER_CONFIG@ --cflags`
csv_analysis_LDFLAGS = `@PRVPARSER_CONFIG@ --libs`
endif
csv_analysis_LDADD   = libspectral.la

compare_signals_SOURCES = \
  compare-signals.c
compare_signals_LDADD   = libspectral.la

dist_bin_SCRIPTS = plot-signal

