#!/usr/bin/perl
#
#  usage: ./compute_shifts.pl trace.prv
#  generates: file with offsets needed to synchronize across all processes
#             the start time of the last useful state
#


open(Y,$ARGV[0]);

$maxprocess=0;
$maxtime=0;

while ($line=<Y>) {
  chop ($line);
  @fields=split(/:/,$line);
  if (($fields[0] == 1) && ($fields[7]==1)) {
	if($fields[5]<$fields[6]) {
    $process = $fields[1];
    $time = $fields[5];
    if ($process > $maxprocess) {$maxprocess = $process;};
    if ($time > $maxtime) {$maxtime = $time;};
    $tstart[$process] = $time;
	}
  } 
}

#print "max time = ".$maxtime."\n" ;

for ($i=1; $i <=$maxprocess; $i++){
    $delta = $maxtime - $tstart[$i];
    print $delta."\n" ;
    
}

close(Y);

