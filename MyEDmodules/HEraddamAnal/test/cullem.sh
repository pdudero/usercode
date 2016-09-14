#!/bin/bash
for file in _HE*/*.txt
do
    lines=`cat $file |wc -l`
    if [ $lines -lt 29 ] 
    then
	echo $lines
	rm $file
    fi
done
