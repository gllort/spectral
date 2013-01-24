MAKEFILE=Makefile
#PLACE: MN, laptop 
PLACE=laptop
EXTRAE_HOME=/opt/cepbatools/extrae

all: all-gcc all-OMP all-OmpSs
all-gcc: trace2tracelib trace2tracelibTrace bin-all
all-OMP: trace2tracelibOMP trace2tracelibTraceOMP bin-allOMP
all-OmpSs: trace2tracelibOMPSs trace2tracelibTraceOMPSs bin-allOMPSs

#make all binaries and place them on ./bin
bin-all: cutterBin traceFilterBin
	cp ./cutter/cutter ./bin
	cp ./trace_filter/trace_filter ./bin

bin-allOMP: cutterBin traceFilterBinOMP
	cp ./cutter/cutter ./bin
	cp ./trace_filter/trace_filter ./bin
	
bin-allOMPSs: cutterBinOMPSs traceFilterBinOMPSs
	cp ./cutter/cutter ./bin
	cp ./trace_filter/trace_filter ./bin

trace2tracelib: cutterLib traceFilterLib
	ar -q libtrace2trace.a cutter/cutter.o trace_filter/trace_filter.o
	mv libtrace2trace.a ./lib

trace2tracelibOMP: cutterLib traceFilterLibOMP
	ar -q libtrace2traceOpenMP.a cutter/cutter.o trace_filter/trace_filter.o
	mv libtrace2traceOpenMP.a ./lib

trace2tracelibOMPSs: cutterLibOMPSs traceFilterLibOMPSs
	ar -q libtrace2traceOmpSs.a cutter/cutter.o trace_filter/trace_filter.o
	mv libtrace2traceOmpSs.a ./lib

trace2tracelibTrace: cutterLibTrace traceFilterLibTrace
	ar -q libtrace2traceTraced.a cutter/cutter.o trace_filter/trace_filter.o
	mv libtrace2traceTraced.a ./lib
	
trace2tracelibTraceOMP: cutterLibTrace traceFilterLibTraceOMP
	ar -q libtrace2traceTracedOpenMP.a cutter/cutter.o trace_filter/trace_filter.o
	mv libtrace2traceTracedOpenMP.a ./lib
	
trace2tracelibTraceOMPSs: cutterLibTraceOMPSs traceFilterLibTraceOMPSs
	ar -q libtrace2traceTracedOmpSs.a cutter/cutter.o trace_filter/trace_filter.o
	mv libtrace2traceTracedOmpSs.a ./lib


cutterBin: cutter//$(MAKEFILE)
	cd cutter && $(MAKE) -f $(MAKEFILE) cutter_$(PLACE)
cutterLib: cutter//$(MAKEFILE)
	cd cutter && $(MAKE) -f $(MAKEFILE) cutter_$(PLACE)_lib
cutterLibTrace: cutter//$(MAKEFILE)
	cd cutter && $(MAKE) -f $(MAKEFILE) cutter_$(PLACE)_lib_trace EXTRAE_HOME=$(EXTRAE_HOME)
cutterBinOMPSs: cutter//$(MAKEFILE)
	cd cutter && $(MAKE) -f $(MAKEFILE) cutterOMPSs_$(PLACE)
cutterLibOMPSs: cutter//$(MAKEFILE)
	cd cutter && $(MAKE) -f $(MAKEFILE) cutterOMPSs_$(PLACE)_lib
cutterLibTraceOMPSs: cutter//$(MAKEFILE)
	cd cutter && $(MAKE) -f $(MAKEFILE) cutterOMPSs_$(PLACE)_lib_trace
cutter-clean: cutter//$(MAKEFILE)
	cd cutter && $(MAKE) -f $(MAKEFILE) clean
	
	
traceFilterBin: trace_filter//$(MAKEFILE)
	cd trace_filter && $(MAKE) -f $(MAKEFILE) trace_filter_$(PLACE)
traceFilterBinOMP: trace_filter//$(MAKEFILE)
	cd trace_filter && $(MAKE) -f $(MAKEFILE) trace_filterOMP_$(PLACE)
traceFilterBinOMPSs: trace_filter//$(MAKEFILE)
	cd trace_filter && $(MAKE) -f $(MAKEFILE) trace_filterOMPSs_$(PLACE)
traceFilterLib: trace_filter//$(MAKEFILE)
	cd trace_filter && $(MAKE) -f $(MAKEFILE) trace_filter_$(PLACE)_lib
traceFilterLibOMP: trace_filter//$(MAKEFILE)
	cd trace_filter && $(MAKE) -f $(MAKEFILE) trace_filterOMP_$(PLACE)_lib
traceFilterLibOMPSs: trace_filter//$(MAKEFILE)
	cd trace_filter && $(MAKE) -f $(MAKEFILE) trace_filterOMPSs_$(PLACE)_lib
traceFilterLibTrace: trace_filter//$(MAKEFILE)
	cd trace_filter && $(MAKE) -f $(MAKEFILE) trace_filter_$(PLACE)_lib_trace EXTRAE_HOME=$(EXTRAE_HOME)
traceFilterLibTraceOMP: trace_filter//$(MAKEFILE)
	cd trace_filter && $(MAKE) -f $(MAKEFILE) trace_filterOMP_$(PLACE)_lib_trace EXTRAE_HOME=$(EXTRAE_HOME)
traceFilterLibTraceOMPSs: trace_filter//$(MAKEFILE)
	cd trace_filter && $(MAKE) -f $(MAKEFILE) trace_filterOMPSs_$(PLACE)_lib_trace
traceFilter-clean: trace_filter//$(MAKEFILE)
	cd trace_filter && $(MAKE) -f $(MAKEFILE) clean


trace2tracelib-clean:
	rm -f ./lib/*.a ./lib/*~
bin-clean:
	rm -f ./bin/*
clean: trace2tracelib-clean bin-clean cutter-clean traceFilter-clean
	rm -f *.o *~
