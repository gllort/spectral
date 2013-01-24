#!/bin/bash


#////////////////// Parameters or global variables //////////////////
#argumentos predefinidos
N=1						# Number of times you want to execute the program
PROG='programa'					# Program name
ORIGINAL='out.programa'				# output of program
out=/tmp/out.$$					# Temporal file where you save the execution results
out_times=/tmp/out.times.$PROG.$$		# Temporal file where you save the time results
original_time="71.82"
PARAMS=""
trace="64"
optionApp="CPUDurBurst"
option="test"
pathToDirTrace=""
dirTrace=""
traceName=""
gprof="2static"
time=""
compile="CC_MODE=gcc"
traceMode="extrae"

#paths
SPECTRAL_HOME="./"
FILTERS_HOME="/home/user/TRACE2TRACE/bin"
TRACE16PATH="/home/user/bt.B.16/"
TRACE64PATH="/home/user/bt.C.64/"
TRACE16=$TRACE16PATH"bt.B.16.prv"
TRACE64=$TRACE64PATH"bt.C.64.prv"
TRACE_ORIGINAL="/home/user/Original/"

#variables extrae y OmpSs 
EXTRAE_CONFIG_FILE=extrae.xml
EXTRAE_HOME=/opt/extrae
UNWIND_HOME=/opt/unwind
#EXTRAE_FUNCTIONS=1
EXTRAE_LABELS=$PWD/labels.data
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${EXTRAE_HOME}/lib/:/usr/local/papi-3.7.2/lib/:${UNWIND_HOME}/lib/:/aplic/Computational/PM/unstable/lib
PATH=$PATH:/aplic/Computational/PM/unstable/bin
OMP_NUM_THREADS=4
NX_INSTRUMENTATION=extrae

#////////////////// Fuctions //////////////////
function UsageFunction()
{
	# Usage function, it writes what does the script with each parametre and exits with -1
	#echo "ERROR: el parametro $1 no es valido"
	echo ""
	echo "Usage: ./test.sh OPTIONS"
	echo "      OPTIONS: las siguientes opciones se pueden ejecutar de manera desordenada."
	echo "               NOTA: si no se especifica -option, -traceName y -time se ejecutara el programa como"
	echo "                     default la opcion CPUDurBurst, la traza b.C.64 y el tiempo 71.82 de b.C.64."
	echo "                     Y del resto de opciones como default se ejecutara -test."
	echo ""
	echo "             -test: ejecucion del programa y comparacion si esta es correcta. Se calcula el tiempo de mejora."
	echo "             -gprof [static/nostatic/2static/]: gprof de optim."
	echo "                    Tambien nos genera el grafo de la ejecucion."
	echo "                    Se compila el codigo en estatico (static), no estatico (nostatic), y,"
	echo "                    tanto estatico y no estatico(2static). Como default 2static (./test.sh -gprof)."
	echo "             -exec: ejecucion normal del programa."
	echo "             -trace [extrae/ompss/]: ejecucion normal del programa generando una traza (optim.prv) para Paraver."
	echo "             -debug: ejecucion del programa y comparacion si esta es correcta con la compilacion en"
	echo "                    modo debug, genera todos los archivos intermedios y comprueba"
	echo "                    que se ha ejecutado correctamente."
	echo "             -gdb: ejecucion del programa en gdb para debugar el programa."
	echo "             -traceName [path/16/64]: introducir el path de la traza (/path/ALYA/trace.prv), ha de tener el nombre"
	echo "                    de la traza en su carpeta. Son posibles los valores 64 y 16 como alias de las trazas"
	echo "                    bt.B.16.prv y bt.C.64 respectivamente."
	echo "             -option [BW/IPC/MPIp2p/CPUBurst/CPUDurBurst]: introduce las opciones BW, IPC, MPIp2p," 
	echo "                    CPUBurst o CPUDurBurstal al programa."
	echo "             -time segs: para -test especifica que ha de comprobar la mejora a partir de segs segundos."
	echo "             -ompss [NUM cpus/]: compilar con OmpSs. Se puede especificar el numero de CPUs (./test.sh -ompss 8),"
	echo "                    sino se especifica (./test.sh -ompss) se utilizara como default 4 CPUs."
	echo "             -help: salida de este USAGE."
	echo ""
	echo "       Ejemplos:"
	echo "             ./test.sh -trace -traceName 16 --> genera la traza de ejecucion a la traza bt.B.16.prv"
	echo "             ./test.sh -time 104.01 -traceName /tmp/ALYA/trace.prv -test --> hace el test del programa"
	echo "                    con la traza /tmp/ALYA/trace.prv y comprueba la mejora con el tiempo 104.01"
	exit -1
	
}

function div()
{
	#function to calculate divisions for floats
	IFS='/'; 
	#scale default 20 decimals
	#echo "scale=4; $*" |bc -l
	echo "$*" |bc -l
}

function mul()
{
	#function to calculate divisions for floats
	IFS='*'; 
	#echo "scale=4; $*" |bc -l
	echo "$*" |bc -l
}

function add()
{
	#function to calculate additions for floats
	IFS='+';
	echo "$*" | bc -l 
}

function sub()
{
	#function to calculate additions for floats
	IFS='-';
	echo "$*" | bc -l 
}


function exectProgramDIR()
{
	#pre: $N, $PROG, $PARS, $out, $out_times, $original_path, $original_files has to be initialized
	#	@param1 final trace path
	#	@param2 final trace file
	#post: executes $N times: /usr/bin/time -f "%e" ./$PROG $PARS> out.$$ 2>> $out_times , and it checks its correct execution with the file out.$PROG.$PARS
	#	the differences of exectProgramDIR is that this one leave the elapsed time output in a line in $out_times for each execution
	
	i=0
	while (test $i -lt $N)
	do
		echo ">Execution" $i
		/usr/bin/time -f "%e" './'$PROG $PARAMS > $out 2>> $out_times
		echo .-Checking execution $i
		original_file=$original_path"out."$PROG
		if (test -f "$original_file")
		then
			if ! (cmp "$original_file" $out)
			then
				#echo Correct Execution >> $out_times
				#echo >> $out_times
			#else
				echo Wrong Execution
				echo Compare $original_file to $out
				#exit -1
			fi
		else
			echo There is not output file $original_file to compare with $out
		fi
		
		
		for file in $original_files
		do
			if (test -f "$original_path$file")
			then
				if ! (cmp "$original_path$file" $file)
				then
					echo Wrong Execution
					echo Compare $original_path$file to $file
					#exit -1
				fi
			else
				echo There is not output file $original_path$file to compare with $file
			fi
		done
		
		for file in $2
		do
			if (test -f "$original_path$file")
			then
				if ! (cmp "$original_path$file" "$1$file")
				then
					echo Wrong Execution
					echo Compare $original_path$file to $1$file
					#exit -1
				fi
			else
				echo There is not output file $original_path$file to compare with $1$file
			fi
		done
		
		#rm -f $out
		i=`expr $i + 1`
	done
}

function exectOPT()
{
	# @param1 program name
	# @param2 original time
	# @param3 dir de archivo original out.programa
	# @param4 archivos extra originales en @param3
	# @param5 final trace path
	# @param6 final trace file
	# @param7 1 if debug mode
	
	PROG=$1
	out_times=/tmp/out.times.$PROG.$$
	original_time=$2
	original_path=$3
	original_files="$4"
	make clean;
	if (test $7 -eq "1")
	then
		make TEST_MODE=1 DEBUG_MODE=1 $compile
	else
		make TEST_MODE=1 $compile
	fi
		
	echo "In process..."	
	
	exectProgramDIR $5 "$6"
	
	#get the average elapsed time from $out_times
	return_value=0
	while read line
	do 
		return_value=`add $return_value  $line`
	done < $out_times
	
	return_value=`div $return_value  $N`
	
	aux=`sub $original_time $return_value`
	echo "Ganancia:$aux tiempo actual:$return_value tiempo original: $original_time"
	
	rm -f $out_times
	
}


function exectGprof()
{
	# @param1 program name
	# @param2 how to compile de code:
	#		static: compile with -static
	#		nostatic: compile without -static
	# inicialized $PARAMS
	
	PROG=$1
	
	make clean;
	make GPROF=$2 $compile
	
	echo "In process with $2..."
	
	mkdir gprofFiles > /dev/null 2>&1
	
	./$PROG $PARAMS &> /dev/null 
	date=`date "+%y%m%d%H%M%S"`
	gprof -b -p ./$PROG > ./gprofFiles/out.gprof.$PROG.$date.b.$2
	gprof -l -p ./$PROG > ./gprofFiles/out.gprof.$PROG.$date.l.$2
	gprof ./$PROG > ./gprofFiles/out.gprof.$PROG.$date.all.$2
	gprof $PROG | ./cgprof.sh -Tps -o ./gprofFiles/graphGprof.$PROG.$date.$2.ps
}

function esNatural()
{
	#pre: $1 string
	#post: return si es un numero natural $1
	#	-0: no es natural
	#	-1: es natural
	
	case $1 in
		*[!0-9]*)
			#no es natural
			echo 0;
		;;
		*)
			#es natural
			echo 1;
		;;
	esac 
}
#////////////////// Start script code (the Main) //////////////////

#--------------- get arguments ---------------
n=$#
i=0
args="$@"

#while getopts :tx:d: opt --> usar set -- `getopt -n$0 -u -a --longoptions="depth: adddays: topN:" "h" "$@"` || usage[ $# -eq 0 ] && usage

while ( test $i -lt $n)
do
	#mirar argumentos
	case $1 in
		"-test")
			option="test"
			
		;;
		"-gprof")
			#posibilidades:
			#	-gprof
			#	-gprof 2static
			#	-gprof static
			#	-gprof nostatic
			
			option="gprof"
			
			#mirar si han especificado cual hacer (default 2static)
			j=`expr $i + 1`
			
			if [[ ($j -lt $n) && ("${2:0:1}" != "-") ]]
			then
				#es una especificacion el siguiente parametro, asi que abanzamos
				i=`expr $i + 1`
				gprof=$2
				shift
				
				if [[ ("$gprof" != "static") &&  ("$gprof" != "nostatic") &&  ("$gprof" != "2static") ]]
				then
					echo "ERROR: La opcion -gprof no existe la especificacion $gprof."
					UsageFunction
				fi
			fi
		;;
		"-exec")
			option="exec"
		;;
		"-trace")
			#posibilidades
			#	-trace
			#	-trace extrae
			#	-trace ompss
			option="trace"
			
			j=`expr $i + 1`
			
			if [[ ($j -lt $n) && ("${2:0:1}" != "-") ]]
			then
				#es una especificacion el siguiente parametro, asi que abanzamos
				i=`expr $i + 1`
				traceMode=$2
				shift
				
				if [[ "$traceMode" != "extrae"  &&  "$traceMode" != "ompss" ]]
				then
					echo "ERROR: La opcion -trace no existe el parametro "$traceMode
					UsageFunction
				fi
			fi
		;;
		"-debug")
			option="debug"
		;;
		"-gdb")
			option="gdb"
		;;
		"-traceName")
			#posibilidades:
			#	-traceName pathToTrace
			#	-traceName 16
			#	-traceName 64
			
			#abanzamos ya que necesita parametros extra
			i=`expr $i + 1`
			
			if(test $i -lt $n)
			then
				#get parametro
				trace=$2
				shift
				
				if [[ "$trace" != "64"  &&  "$trace" != "16" ]]
				then
					path=$trace
					trace="other"
					
					if [ -f $path ]
					then
						# ejemplo path es /1/2/3/trace.prv
						
						#get dir --> /1/2/3
						dir=${path%/*}
						#get path to dir --> /1/3
						pathToDirTrace=${dir%/*}
						#get name dir --> 3
						dirTrace=${dir##*/}
						#get filename --> trace.prv
						traceName=${path##*/}
						
						#el caso /1/trace.prv peta
						if(test "$pathToDirTrace" == "")
						then
							#en este caso poner root
							pathToDirTrace="/"
						fi
						
						#el caso /trace.prv no lo queremos, necesitamos una carpeta
						if(test "$dirTrace" == "")
						then
							echo "ERROR: fichero $path necesitamos una carpeta y que no este situado en root /traza.prv, sino algo como /path/trace.prv"
							exit 0;
						fi
					else
						echo "ERROR: fichero $path no existe o es un directorio."
						exit 0;
					fi
				fi
				#else --> -traceName 16 o -traceName 64 (parametros validos)
				
				
			else
				#le falta el segundo parametro
				echo "ERROR: La opcion -traceName necesita el path de la traza."
				UsageFunction
			fi
		;;
		"-option")
			#posibilidades:
			#	-option BW
			#	-option IPC
			#	-option MPIp2p
			#	-option CPUBurst
			#	-option CPUDurBurst
			
			#abanzamos ya que necesita un parametro extra
			i=`expr $i + 1`
			
			if(test $i -lt $n)
			then
				#get parametro
				optionApp=$2
				shift
				
				if [[ "$optionApp" != "BW"  &&  "$optionApp" != "IPC"  &&  "$optionApp" != "MPIp2p"  &&  "$optionApp" != "CPUBurst"  &&  "$optionApp" != "CPUDurBurst" ]]
				then
					echo "ERROR: La opcion -option no existe el parametro "$optionApp
					UsageFunction
				fi
				#else --> -option [BW/IPC/MPIp2p/CPUBurst/CPUDurBurst] (parametros validos)
				
			else
				#le falta el segundo parametro
				echo "ERROR: La opcion -option necesita la especificacion de la opcion"
				UsageFunction
			fi
		;;
		"-time")
			#-time TIME --> ej: -time 15.28
			i=`expr $i + 1`
			
			if(test $i -lt $n)
			then
				#get parametro
				time=$2
				shift
			else
				#le falta el segundo parametro
				echo "ERROR: La opcion -time necesita la especificacion del tiempo."
				UsageFunction
			fi
		;;
		"-ompss")
			#posibilidades
			#	-ompss
			#	-ompss NUMcpus --> ej: -ompss 8
			
			compile="CC_MODE=ompss"
						
			j=`expr $i + 1`
			
			if [[ ($j -lt $n) && ("${2:0:1}" != "-") ]]
			then
				#es una especificacion el siguiente parametro, asi que abanzamos
				i=`expr $i + 1`
				OMP_NUM_THREADS=$2
				shift
				
				if(test `esNatural $OMP_NUM_THREADS` -eq 0)
				then
					#no es un numero natural
					echo "ERROR: La opcion -ompss la especificacion del numero de CPUs ha de ser un numero natural y no $OMP_NUM_THREADS."
					UsageFunction					
				fi
			fi
			export LD_LIBRARY_PATH
			export PATH
			export OMP_NUM_THREADS
		;;
		"-help")
			UsageFunction
		;;
		*)
			#parametro erroneo
			echo "ERROR: el parametro $1 no es valido"
			UsageFunction
		;;
	esac
	
	#abanzamos
	shift
	i=`expr $i + 1`
done

if [[ "$traceMode" = "ompss"  &&  "$compile" != "CC_MODE=ompss" ]]
then
	#no se puede tracear ompss si no se compila con ompss
	echo "WARNING: No se puede tracear con el modo -trace ompss si no se compila la aplicacion con OmpSs."
	echo "         Asi que se ha cambiado a cambiado a -ompss."
	compile="CC_MODE=ompss"
	export LD_LIBRARY_PATH
	export PATH
	export OMP_NUM_THREADS
fi

#echo "---------test------------"
#echo "trace: "$trace
#echo "option app: "$optionApp
#echo "option exec: "$option
#echo "path to dir trace: "$pathToDirTrace
#echo "dir trace: "$dirTrace
#echo "trace name: "$traceName
#echo "gprof specif: "$gprof
#echo "time : "$time
#echo "compile: "$compile
#echo "Num CPUs: "$OMP_NUM_THREADS
#echo "tracemode: "$traceMode
#exit 0

#--------------- execute script ---------------
case $option in
	
	"test")
		
		case $trace in
			"16")
				PARAMS="$TRACE16 $optionApp"
				dir="bt.B.16"
				outTrace="bt.C.16.prv.$optionApp.filtered.prv bt.C.16.prv.$optionApp.filtered.pcf bt.C.16.prv.$optionApp.filtered.row"
				outTracePath=$TRACE16PATH
				if(test "$time" == "")
				then
					time="13.66"
				fi
			;;
			
			"64")
				PARAMS="$TRACE64 $optionApp"
				dir="bt.C.64"
				outTrace="bt.C.64.prv.$optionApp.filtered.prv bt.C.64.prv.$optionApp.filtered.pcf bt.C.64.prv.$optionApp.filtered.row"
				outTracePath=$TRACE64PATH
				if(test "$time" == "")
				then
					time="71.82"
				fi
			;;
			
			"other")
				PARAMS="$pathToDirTrace/$dirTrace/$traceName $optionApp"
				time="109.07"
				dir="$dirTrace"
				outTrace="$traceName.$optionApp.filtered.prv $traceName.$optionApp.filtered.pcf $traceName.$optionApp.filtered.row"
				outTracePath="$pathToDirTrace/$dirTrace/"
			;;
		esac
		
		export SPECTRAL_HOME
		export FILTERS_HOME
		
		exectOPT "optim" $time "$TRACE_ORIGINAL$dir/" "report.out report.err" $outTracePath "$outTrace" 0
	
	;;
	
	"gprof")
		
		case $trace in
			"16")
				PARAMS="$TRACE16 $optionApp"
			;;
			
			"64")
				PARAMS="$TRACE64 $optionApp"
			;;
			
			"other")
				PARAMS="$pathToDirTrace/$dirTrace/$traceName $optionApp"
			;;
		esac
		
		export SPECTRAL_HOME
		export FILTERS_HOME
		
		
		case $gprof in
			"static")
				exectGprof "optim" "static"
			;;
			
			"nostatic")
				exectGprof "optim" "nostatic"
			;;
			
			"2static")
				exectGprof "optim" "nostatic"
				exectGprof "optim" "static"
			;;
		esac
			
	;;
	
	"exec")
		case $trace in
			"16")
				PARAMS="$TRACE16 $optionApp"
			;;
			
			"64")
				PARAMS="$TRACE64 $optionApp"
			;;
			
			"other")
				PARAMS="$pathToDirTrace/$dirTrace/$traceName $optionApp"
			;;
		esac
		
		export SPECTRAL_HOME
		export FILTERS_HOME
		
		make clean;
		make $compile
		
		./optim $PARAMS
	;;
	
	"trace")
		case $trace in
			"16")
				PARAMS="$TRACE16 $optionApp"
			;;
			
			"64")
				PARAMS="$TRACE64 $optionApp"
			;;
			
			"other")
				PARAMS="$pathToDirTrace/$dirTrace/$traceName $optionApp"
			;;
		esac
		
		export SPECTRAL_HOME
		export FILTERS_HOME
		
		if (test "$traceMode" == "ompss")
		then
			export NX_INSTRUMENTATION
		else
			export EXTRAE_CONFIG_FILE
			export EXTRAE_HOME
			export UNWIND_HOME
			export LD_LIBRARY_PATH
			export EXTRAE_FUNCTIONS
		fi
			
		export EXTRAE_LABELS
		
		
		make clean;
		make TRACE=$traceMode $compile
		rm -f /tmp/logTrace*
		./optim $PARAMS > /tmp/logTrace$$
	;;
	
	"debug")
		
		case $trace in
			"16")
				PARAMS="$TRACE16 $optionApp"
				dir="bt.B.16"
				outTrace="bt.C.16.prv.$optionApp.filtered.prv bt.C.16.prv.$optionApp.filtered.pcf bt.C.16.prv.$optionApp.filtered.row"
				outTracePath=$TRACE16PATH
				if(test "$time" == "")
				then
					time="13.66"
				fi
			;;
			
			"64")
				PARAMS="$TRACE64 $optionApp"
				dir="bt.C.64"
				outTrace="bt.C.64.prv.$optionApp.filtered.prv bt.C.64.prv.$optionApp.filtered.pcf bt.C.64.prv.$optionApp.filtered.row"
				outTracePath=$TRACE64PATH
				if(test "$time" == "")
				then
					time="71.82"
				fi
			;;
			
			"other")
				PARAMS="$pathToDirTrace/$dirTrace/$traceName $optionApp"
				time="109.07"
				dir="$dirTrace"
				outTrace="$traceName.$optionApp.filtered.prv $traceName.$optionApp.filtered.pcf $traceName.$optionApp.filtered.row"
				outTracePath="$pathToDirTrace/$dirTrace/"
			;;
		esac
		
		export SPECTRAL_HOME
		export FILTERS_HOME
		
		exectOPT "optim" $time "$TRACE_ORIGINAL$dir/" "KK out.txt out2.txt tall.txt signal.samp.txt signal.txt PPP2 signal3.txt outin.txt signalx2.txt px.txt Freqx.txt Freq2x.txt output.txt Tcorrect sin.txt outin.samp.txt Crosscorrelation.txt signalt2.txt report.out report.err" $outTracePath "$outTrace" 1
	
	;;
	
	"gdb")
		case $trace in
			"16")
				PARAMS="$TRACE16 $optionApp"
			;;
			
			"64")
				PARAMS="$TRACE64 $optionApp"
			;;
			
			"other")
				PARAMS="$pathToDirTrace/$dirTrace/$traceName $optionApp"
			;;
		esac
		
		export SPECTRAL_HOME
		export FILTERS_HOME
		
		make clean;
		make GDB_MODE=1 $compile
		
		gdb --args ./optim $PARAMS
	;;
esac

exit 0
