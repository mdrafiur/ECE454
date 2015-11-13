#!/bin/bash

#measure the accuracy of the output for each randtrack program

make clean
make
./randtrack 1 50 > rt1.out
sort -n rt1.out > rt1.outs
: '
./randtrack_global_lock 4 50 > rt2.out
sort -n rt2.out > rt2.outs
./randtrack_tm 4 50 > rt3.out
sort -n rt3.out > rt3.outs
#'
./randtrack_list_lock 4 50 > rt4.out
sort -n rt4.out > rt4.outs
#diff rt1.outs rt2.outs
#diff rt1.outs rt3.outs
diff rt1.outs rt4.outs
