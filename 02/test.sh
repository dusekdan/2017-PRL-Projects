#!/bin/bash
# Zdrojovy kod prevzat z http://www.fit.vutbr.cz/~ikalmar/PRL/odd-even-trans/test

if [ $# -lt 1 ];then 
    numbers=10;
else
    numbers=$1;
fi;

processes=$(($numbers+1))

mpic++ --prefix /usr/local/share/OpenMPI -o es es.cpp 1> /dev/null 2> /dev/null
dd if=/dev/random bs=1 count=$numbers of=numbers 1> /dev/null 2> /dev/null
mpirun --prefix /usr/local/share/OpenMPI -np $processes es

rm -f es numbers 1> /dev/null 2> /dev/null

