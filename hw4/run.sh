#!/bin/bash

#make a clean new copy of each file
make clean
make

#measure the accuracy of the output for each randtrack program
: '
./randtrack 1 50 > rt1.out
sort -n rt1.out > rt1.outs
./randtrack_global_lock 4 50 > rt2.out
sort -n rt2.out > rt2.outs
./randtrack_tm 4 50 > rt3.out
sort -n rt3.out > rt3.outs
./randtrack_list_lock 4 50 > rt4.out
sort -n rt4.out > rt4.outs
./randtrack_element_lock 4 50 > rt5.out
sort -n rt5.out > rt5.outs
./randtrack_reduction 4 50 > rt6.out
sort -n rt6.out > rt6.outs
diff rt1.outs rt2.outs
diff rt1.outs rt3.outs
diff rt1.outs rt4.outs
diff rt1.outs rt5.outs
diff rt1.outs rt6.outs
#'
: '
#measure the runtime of each randtrack program with skip rate = 50
number=0
while [ $number -lt 5 ]; do
	number=$((number + 1))
	#change this according which randtrack you are measuring
	#/usr/bin/time --verbose ./randtrack 1 50 > rt1.out
	#/usr/bin/time --verbose ./randtrack_global_lock 4 50 > rt2.out
	#/usr/bin/time --verbose ./randtrack_tm 4 50 > rt3.out
	#/usr/bin/time --verbose ./randtrack_list_lock 4 50 > rt4.out
	#/usr/bin/time --verbose ./randtrack_element_lock 4 50 > rt5.out
	#/usr/bin/time --verbose ./randtrack_reduction 4 50 > rt6.out
	#sleep 2
done
#'
#measure the runtime of each randtrack program with skip rate = 100
number=0
while [ $number -lt 5 ]; do
	number=$((number + 1))
	#change this according which randtrack you are measuring
	#/usr/bin/time --verbose ./randtrack 1 100 > rt1.out
	#/usr/bin/time --verbose ./randtrack_global_lock 4 100 > rt2.out
	#/usr/bin/time --verbose ./randtrack_tm 4 100 > rt3.out
	#/usr/bin/time --verbose ./randtrack_list_lock 4 100 > rt4.out
	#/usr/bin/time --verbose ./randtrack_element_lock 4 100 > rt5.out
	/usr/bin/time --verbose ./randtrack_reduction 4 100 > rt6.out
	#sleep 2
done

#Can check if a particular output is wrong for one rand
: '
./randtrack 1 50 > rt1.out
sort -n rt1.out > rt1.outs
sort -n rt6.out > rt6.outs
diff rt1.outs rt6.outs
#'
