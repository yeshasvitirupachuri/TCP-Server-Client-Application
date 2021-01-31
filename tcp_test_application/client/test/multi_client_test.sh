#!/bin/sh
## input arguments : client executable, number of clients
c=1
clintCount=$2
echo $1
while [ $c -le $clintCount ]
do
    ./$1
    c=$((c+1))
done
