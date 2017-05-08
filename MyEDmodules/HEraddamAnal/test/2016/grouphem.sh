#!/bin/bash
if [ $# -lt 1 ]
then
    echo "Usage: $0 hem.txt"
    exit
fi
sort -k4n -k2n -k1n $1 | awk '{print > "fcamps2016_"$4".txt"}'
#
# for (( i=-16; i<=-16; i++ ))
for (( i=-29; i<=-16; i++ ))
do
    mkdir _HE$i
    mv `grep -l "HE $i" fcamps2016_11*.txt` _HE$i
done
