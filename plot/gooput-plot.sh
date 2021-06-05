#! /bin/sh
data_dir=linux-reno/
suffix=goodput
file1=${data_dir}10.1.1.1_49153_10.1.1.2_5001_${suffix}.txt
file2=${data_dir}10.1.1.1_49154_10.1.1.2_5001_${suffix}.txt
output=${suffix}
gnuplot<<!
set key bottom right
set xlabel "time/s" 
set ylabel "rate/kbps"
set xrange [0:200]
set yrange [500:2500]
set grid
set term "png"
set output "${output}.png"
plot "${file1}" u 1:2 title "flow1" with linespoints lw 2,\
"${file2}" u 1:2 title "flow2" with linespoints lw 2
set output
exit
!


