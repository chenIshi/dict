reset
set ylabel 'time(micro sec)'
set xlabel 'data count'
set title 'average search time per 100 data'
set term png enhanced font 'Verdana,10'
set output 'search_time.png'

plot \
"search_time_bloom.txt" using 1:2 with linespoints linewidth 2 title "With Bloom", \
"search_time_wobloom.txt" using 1:2 with linespoints linewidth 2 title "Without Bloom" \
