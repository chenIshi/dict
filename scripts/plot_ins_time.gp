reset
set ylabel 'time(micro sec)'
set xlabel 'data count(ten thousand)'
set title 'insertion time per tens of thousand'
set format x ""
set term png enhanced font 'Verdana,10'
set output 'insertion_time.png'

plot \
"ins_time_bloom.txt" using 1:2 with linespoints linewidth 2 title "With Bloom", \
"ins_time_wobloom.txt" using 1:2 with linespoints linewidth 2 title "Without Bloom" \