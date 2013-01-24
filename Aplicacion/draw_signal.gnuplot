
set xlabel 'Time'
set ylabel 'Duration'
plot "signal.txt" using 1:3 with steps 
pause -1 "Hit return to continue..."

