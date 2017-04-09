#!/bin/bash
# Zdrojovy kod prevzat z http://www.fit.vutbr.cz/~ikalmar/PRL/odd-even-trans/test

if [ $# -lt 1 ];then 
    numbers=28;
else
    numbers=$1;
fi;

# Measure 10 processors 30 times
for i in `seq 1 30`
do
	processes=$(($numbers+1))
	mpic++ --prefix /usr/local/share/OpenMPI -o es es.cpp #1> /dev/null 2> /dev/null
	dd if=/dev/random bs=1 count=$numbers of=numbers 1> /dev/null 2> /dev/null
	mpirun --prefix /usr/local/share/OpenMPI -np $processes es >> n28

	rm -f es numbers

	echo "Round $i run now" 
done


#echo "Sum for 10 processors and 30 iterations:"
#(cat n | tr "\012" "+"; echo "0") | bc


# Measure 20 processors 30 times
#for i in `seq 1 30`
#do
#	numbers2=20
#	processes2=21
#	mpic++ --prefix /usr/local/share/OpenMPI -o es es_dan.cpp #1> /dev/null 2> /dev/null
#	dd if=/dev/random bs=1 count=$numbers2 of=numbers 1> /dev/null 2> /dev/null
#	mpirun --prefix /usr/local/share/OpenMPI -np $processes2 es >> n2

#	rm -f es numbers

#	echo "Round $i run now" 
#done