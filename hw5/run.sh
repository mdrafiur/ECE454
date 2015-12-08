#!/bin/bash

#make a clean new copy of each file
make clean
make


#measure the runtime of each interation for 5 times and take the average
number=0
while [ $number -lt 5 ]; do
	number=$((number + 1))
	/usr/bin/time -f "%e real" ./gol 10000 inputs/1k.pbm outputs/1k.pbm
done

diff outputs/1k.pbm outputs/1k_verify_out.pbm
echo TestDone
