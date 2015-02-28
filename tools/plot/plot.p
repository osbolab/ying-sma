#!/usr/local/bin/gnuplot -persist

set terminal postscript eps enhanced rounded  color font 'Arial, 24'

set style line 1 lt 1 lc rgb "#A00000" lw 2 pt 7 ps 1.5
set style line 2 lt 1 lc rgb "#00A000" lw 2 pt 11 ps 1.5
set style line 3 lt 1 lc rgb "#5060D0" lw 2 pt 9 ps 1.5
set style line 4 lt 1 lc rgb "#0000A0" lw 2 pt 8 ps 1.5
set style line 5 lt 1 lc rgb "#D0D000" lw 2 pt 13 ps 1.5
set style line 6 lt 1 lc rgb "#00D0D0" lw 5 pt 12 ps 1.5
set style line 7 lt 1 lc rgb "#B200B2" lw 2 pt 5 ps 1.5


set output 'scheduling_delay.eps'
set xlabel 'time'
set grid
set xtics nomirror
#set xrange[0:600]
#set xtics ('0' 0, '200' 200, '400' 400, '600' 600)
set ytics nomirror
set style line 80 lt 0 lc rgb "#808080"
set border 3 back ls 80 
set style line 81 lt 0 lc rgb "#808080" lw 0.5
set grid back ls 81
set size 1, 1
set xlabel 'time (seconds)'
set ylabel 'scheduling delay (microseconds)'
set key left
set style data points
plot './delay_output' u 0:1 w p ls 1 t 'scheduling delay'

set output 'utility.eps'
set grid
set xtics nomirror
set ytics nomirror
set size 1, 1
set xlabel 'time'
set ylabel 'utility'
set key left
plot './utility_output' u 0:1 w l ls 7 t 'utility'

set output 'utilization.eps'
set style fill pattern 4 bo
set grid
set xtics nomirror
set ytics nomirror
set size 1,1
set xlabel 'time (seconds)'
set ylabel 'cache utilization'
set key left
plot './utilization_output' u 0:1 w l ls 6 t 'cache utilization'#, \
#'./utilization_output' u  0:2 w l ls 5 t 'store utilization'

set output 'blockevent.eps'
set style fill pattern 4 bo
set grid
set xtics nomirror
set ytics nomirror
set size 1,1
set xlabel 'time (seconds)'
set ylabel '# of broadcast'
set key left
plot './blockevent.txt' u 1:2 w l ls 7 t 'block broadcast'


set output 'bandwidth.eps'
set grid
set xtics nomirror
set ytics nomirror
set size 1, 1
set xlabel 'time (seconds)'
set ylabel 'bytes per second'
set key right

stats './bandwidth.txt' every ::1 using 2 nooutput
total = int(STATS_sum)
stats './bandwidth.txt' every ::1 using 3 nooutput
total_r = int(STATS_sum)
stats './bandwidth.txt' every ::1 using 4 nooutput
total_b = int(STATS_sum)
stats './bandwidth.txt' every ::1 using 5 nooutput
total_i = int(STATS_sum)
stats './bandwidth.txt' every ::1 using 6 nooutput
total_m = int(STATS_sum)


plot './bandwidth.txt' u 1:2 with filledcurve x1 t sprintf('total packets: %d', total), \
     './bandwidth.txt' u 1:3 w l ls 2 t sprintf('request packets: %d', total_r), \
     './bandwidth.txt' u 1:4 w l ls 3 t sprintf('block packets: %d', total_b), \
     './bandwidth.txt' u 1:5 w l ls 4 t sprintf('interest packets: %d', total_i), \
     './bandwidth.txt' u 1:6 w l ls 5 t sprintf('meta bytes: %d', total_m)


